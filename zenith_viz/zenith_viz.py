import logging
import os
import re
import struct
import threading
from abc import ABC
from enum import Enum
from functools import reduce, partial
from typing import Collection, Union, Optional, Tuple, Type, Set, List

import jellyfish
import numpy as np
import pandas as pd
import yaml
import _zenith

directory = os.path.dirname(os.path.abspath(__file__))
color_yaml = directory + "/resources/colors.yml"
shaders = directory + "/shaders"

with open(color_yaml, "r") as f:
    color_lookup = yaml.load(f, yaml.Loader)


class InvalidColorRepresentationError(ValueError):
    def __init__(self, message):
        super().__init__(message)
        self.message = message


class DrawStyles(Enum):
    GL_POINTS = 0
    GL_LINES = 1
    GL_LINE_LOOP = 2
    GL_LINE_STRIP = 3
    GL_TRIANGLES = 4
    GL_TRIANGLE_STRIP = 5
    GL_TRIANGLE_FAN = 6
    GL_QUADS = 7
    GL_QUAD_STRIP = 8
    GL_POLYGON = 9


class ZenithCommon(ABC):
    __num_layers__: int
    __engine__: _zenith.Engine
    __layer_ids__: Set[int]
    __layers__: List[_zenith.GLModel]
    __logger__: logging.Logger

    def __init__(self):
        self.__num_layers__: int = 0
        self.__layer_ids__: Set[int] = set()
        self.__layers__: List[_zenith.GLModel] = []
        self.__logger__: logging.Logger = logging.Logger(__name__)
        self.__logger__.setLevel(logging.INFO)
        self.__string_data__ = []

    def _check_values(
        self,
        x_data: Collection[float],
        y_data: Collection[float],
        z_data: Optional[Collection[float]] = None,
        time_data: Optional[Collection[int]] = None,
    ) -> bool:
        type_is_correct = np.array(x_data).dtype.name in {"float32", "float64"}
        type_is_correct = type_is_correct and np.array(y_data).dtype.name in {
            "float32",
            "float64",
        }
        len_equal = len(x_data) == len(y_data)
        if z_data is not None:
            type_is_correct = type_is_correct and np.array(z_data).dtype.name in {
                "float32",
                "float64",
            }
            len_equal = len_equal and len(z_data) == len(x_data)
        if time_data is not None:
            type_is_correct = type_is_correct and np.array(time_data).dtype.name in {
                "int32",
                "int64",
            }
            len_equal = len_equal and len(time_data) == len(x_data)
        result = type_is_correct and len_equal
        if not result:
            self.__logger__.error("x and y must have the same length")
        return result

    def _check_name(self, name: str):
        is_string = type(name) == str
        len_gt_0 = len(name) > 0
        result = is_string and len_gt_0
        if not result:
            self.__logger__.error("Name must be a non-empty string")
        return result

    def _check_draw_style(self, draw_style: Union[int, DrawStyles]) -> int:
        if type(draw_style) == DrawStyles:
            draw_style = draw_style.value
        if 0 <= draw_style <= 9:
            return draw_style
        else:
            self.__logger__.error(
                "Must pick draw style from the draw_styles Enum -- DEFAULTING TO GL_POINTS"
            )
            return 0

    def show(self) -> bool:
        if threading.current_thread() is not threading.main_thread():
            return False
        self.__engine__.animate()
        return True

    def remove_layer(self, layer_id: int) -> bool:
        return self.__engine__.remove_model(layer_id)

    def check_color_data(self, color_data):
        if color_data is None:
            return np.zeros(3, dtype=np.float32)
        else:
            return color_data

    @staticmethod
    def __construct_color_from_hex__(hex_code: str) -> Tuple[int]:
        length = len(hex_code)
        b = bytes.fromhex(hex_code.replace("#", ""))
        pattern = "B" * int(length / 2)
        result = struct.unpack(pattern, b)
        if len(result) == 3:
            result = tuple([*result, 127])
        return result

    @staticmethod
    def __validate_color_vector_type_and_values__(
        color: Union[Collection[int], Collection[float]],
        type_check: Type,
        lower: Union[int, float],
        upper: Union[int, float],
    ) -> bool:
        type_is_correct = reduce(
            lambda x, y: x and y, map(lambda x: type(x) == type_check, color)
        )
        value_ranges_are_correct = reduce(
            lambda x, y: x and y, map(lambda x: lower <= x <= upper, color)
        )
        length_is_correct = len(color) == 3 or len(color) == 4
        return type_is_correct and value_ranges_are_correct and length_is_correct

    def __validate_color_string_value__(self, color) -> Tuple[int]:
        if re.match(r"^#[0-9a-fA-F]{6}$|^#[0-9a-fA-F]{8}$", color):
            return self.__construct_color_from_hex__(color)
        else:
            if color in color_lookup:
                return self.__construct_color_from_hex__(color_lookup[color].lower())
            else:
                partial_ld = partial(jellyfish.levenshtein_distance, color)
                keys = color_lookup.keys()
                most_similar = sorted(
                    list(zip(keys, map(partial_ld, keys))),
                    key=lambda pair: pair[1],
                )[0]
                if most_similar[1] < 3:  # edit distance is pretty similar...
                    raise InvalidColorRepresentationError(
                        "Couldn't find that color... maybe you meant '{}'?".format(
                            most_similar[0]
                        )
                    )
            raise InvalidColorRepresentationError("No matching color found for input")

    def __validate_and_map_color__(
        self, color: Union[str, Collection[int], Collection[float]]
    ) -> Tuple[int]:
        if isinstance(color, str):
            return self.__validate_color_string_value__(color)
        elif isinstance(color, (tuple, list, np.ndarray, pd.Series)):
            if self.__validate_color_vector_type_and_values__(color, int, 0, 255):
                return tuple(color)
            elif self.__validate_color_vector_type_and_values__(color, float, 0.0, 1.0):
                return tuple(map(lambda x: int(x * 255), color))
            else:
                raise InvalidColorRepresentationError(
                    "color must include either 3 or 4 channels (RGB or RGBA), int range 0-255 float range 0.0 - 1.0"
                )
        else:
            raise InvalidColorRepresentationError(
                "Found no suitable representation of color for given input"
            )

    def __validate_string_data__(
        self, string_data: Optional[Collection[str]], length: int
    ) -> Collection[str]:
        if string_data is None:
            return []
        if len(string_data) == length:
            return string_data
        return []


class Zenith2D(ZenithCommon):
    def __init__(self):
        super().__init__()
        self.__engine__: _zenith.Engine3d = _zenith.Engine3d(shaders)

    def add_layer(
        self,
        x_data: Collection[float],
        y_data: Collection[float],
        name: str,
        draw_style: Union[int, DrawStyles],
        color: Union[str, Collection[int], Collection[float]],
        color_data: Optional[Collection[float]] = None,
        string_data: Optional[Collection[str]] = None,
        picking_enabled: Optional[bool] = False,
    ) -> Union[int, bool]:
        use_color_data = 1 if color_data is not None else 0

        if not self._check_values(x_data, y_data):
            return False
        draw_style = self._check_draw_style(draw_style)
        if not self._check_name(name):
            return False
        string_data = self.__validate_string_data__(string_data, len(x_data))
        self.__string_data__.append(string_data)

        data = np.ravel(np.vstack((x_data, y_data, np.ones(len(x_data)))), order="F")
        color = (
            np.array(self.__validate_and_map_color__(color), dtype=np.float32) / 255.0
        )
        self.__num_layers__ = self.__num_layers__ + 1
        model_id = self.__num_layers__

        model = _zenith.create_gl_model(
            data.astype(np.float32),
            len(x_data),
            3,
            0,
            draw_style,
            str(name),
            color,
            self.check_color_data(color_data),
            use_color_data,
            model_id,
            string_data,
            picking_enabled,
        )
        self.__layers__.append(model)
        self.__layer_ids__.add(model_id)
        return model_id

    def add_animated_layer(
        self,
        x_data: Collection[float],
        y_data: Collection[float],
        time_data: Collection[int],
        color: Union[str, Collection[int], Collection[float]],
        draw_style: Union[int, DrawStyles],
        window_size: int,
        name: str,
        color_data: Optional[Collection[float]] = None,
        string_data: Optional[Collection[str]] = None,
        picking_enabled: Optional[bool] = False,
    ) -> Union[int, bool]:
        use_color_data = 1 if color_data is not None else 0

        if not self._check_values(x_data, y_data, time_data=time_data):
            return False
        draw_style = self._check_draw_style(draw_style)
        if not self._check_name(name):
            return False
        string_data = self.__validate_string_data__(string_data, len(x_data))
        self.__string_data__.append(string_data)
        if window_size < 1 or window_size > len(x_data):
            self.__logger__.error(
                "Window size must be gt than 0 and lte to length of x, y, and time_data"
            )
            return False
        color = (
            np.array(self.__validate_and_map_color__(color), dtype=np.float32) / 255.0
        )

        vertex_data = np.ravel(
            np.vstack((x_data, y_data, np.ones(len(x_data)))), order="F"
        )
        time_array = np.array(time_data, dtype=np.int64)

        self.__num_layers__ = self.__num_layers__ + 1
        model_id = self.__num_layers__

        model = _zenith.create_gl_model_animated(
            vertex_data.astype(np.float32),
            len(x_data),
            3,
            0,
            draw_style,
            window_size,
            window_size,
            time_array,
            name,
            color,
            self.check_color_data(color_data),
            use_color_data,
            model_id,
            string_data,
            picking_enabled,
        )
        model_id = self.__num_layers__
        self.__engine__.add_model(model_id, model)
        self.__layers__.append(model)
        self.__layer_ids__.add(model_id)
        return model_id


class Zenith3D(ZenithCommon):
    def __init__(self):
        super().__init__()
        self.__engine__: _zenith.Engine3d = _zenith.Engine3d(shaders)

    def add_layer(
        self,
        x_data: Collection[float],
        y_data: Collection[float],
        z_data: Collection[float],
        color: Union[str, Collection[int], Collection[float]],
        name: str,
        draw_style: Union[int, DrawStyles],
        color_data: Optional[Collection[float]] = None,
        string_data: Optional[Collection[str]] = None,
        picking_enabled: Optional[bool] = False,
    ) -> Union[int, bool]:
        use_color_data = 1 if color_data is not None else 0

        if not self._check_values(x_data, y_data, z_data):
            return False
        draw_style = self._check_draw_style(draw_style)
        if not self._check_name(name):
            return False
        string_data = self.__validate_string_data__(string_data, len(x_data))
        self.__string_data__.append(string_data)
        data = np.ravel(np.vstack((x_data, y_data, z_data)), order="F")
        color = (
            np.array(self.__validate_and_map_color__(color), dtype=np.float32) / 255.0
        )

        self.__num_layers__ = self.__num_layers__ + 1
        model_id = self.__num_layers__

        model = _zenith.create_gl_model(
            data.astype(np.float32),
            len(x_data),
            3,
            0,
            draw_style,
            str(name),
            color,
            self.check_color_data(color_data),
            use_color_data,
            model_id,
            string_data,
            picking_enabled,
        )

        self.__engine__.add_model(model_id, model)
        self.__layers__.append(model)
        self.__layer_ids__.add(model_id)
        return model_id

    def add_animated_layer(
        self,
        x_data: Collection[float],
        y_data: Collection[float],
        z_data: Collection[float],
        time_data: Collection[int],
        color: Union[str, Collection[int], Collection[float]],
        draw_style: Union[int, DrawStyles],
        window_size: int,
        name: str,
        color_data: Optional[Collection[float]] = None,
        string_data: Optional[Collection[str]] = None,
        picking_enabled: Optional[bool] = False,
    ) -> Union[int, bool]:
        use_color_data = 1 if color_data is not None else 0

        if not self._check_values(x_data, y_data, z_data, time_data):
            return False
        string_data = self.__validate_string_data__(string_data, len(x_data))
        self.__string_data__.append(string_data)
        draw_style = self._check_draw_style(draw_style)
        if not self._check_name(name):
            return False
        if window_size < 1 or window_size > len(x_data):
            self.__logger__.error(
                "Window size must be gt than 0 and lte to length of x, y, and time_data"
            )
            return False
        vertex_data = np.ravel(np.vstack((x_data, y_data, z_data)), order="F")
        time_array = np.array(time_data, dtype=np.int64)
        color = (
            np.array(self.__validate_and_map_color__(color), dtype=np.float32) / 255.0
        )

        self.__num_layers__ = self.__num_layers__ + 1
        model_id = self.__num_layers__

        model = _zenith.create_gl_model_animated(
            vertex_data.astype(np.float32),
            len(x_data),
            3,
            0,
            draw_style,
            window_size,
            window_size,
            time_array,
            name,
            color,
            self.check_color_data(color_data),
            use_color_data,
            model_id,
            string_data,
            picking_enabled,
        )
        self.__engine__.add_model(model_id, model)
        self.__layers__.append(model)
        self.__layer_ids__.add(model_id)
        return model_id
