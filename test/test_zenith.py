from functools import reduce

import numpy as np
from zenith_viz.zenith_viz import Zenith2D, DrawStyles, InvalidColorRepresentationError

plot = Zenith2D()


def test_can_convert_3_channel_hex_code_to_color():
    color = plot.__validate_and_map_color__("#ffffff")
    assert reduce(
        lambda x, y: x and y,
        map(lambda x: x[0] == x[1], zip(tuple([255, 255, 255, 127]), color)),
    )


def test_can_convert_4_channel_hex_code_to_color():
    color = plot.__validate_and_map_color__("#ffffffff")
    assert reduce(
        lambda x, y: x and y,
        map(lambda x: x[0] == x[1], zip(tuple([255, 255, 255, 255]), color)),
    )


def test_color_conversion_fails_on_bad_hex_codes():
    try:
        color = plot.__validate_and_map_color__("#junk")
        assert False
    except InvalidColorRepresentationError as icre:
        assert True
    try:
        color = plot.__validate_and_map_color__("#3f3f")
        assert False
    except InvalidColorRepresentationError as icre:
        assert True
    try:
        color = plot.__validate_and_map_color__("#123456789")
        assert False
    except InvalidColorRepresentationError as icre:
        assert True


def test_color_works_on_known_color_name():
    color = plot.__validate_and_map_color__("CornflowerBlue")
    assert color[0] == 100
    assert color[1] == 149
    assert color[2] == 237
    assert color[3] == 127


def test_color_recommends_similar_name_in_exception():
    try:
        color = plot.__validate_and_map_color__("cornflowerblue")
    except InvalidColorRepresentationError as icre:
        assert "CornflowerBlue" in icre.message
    try:
        color = plot.__validate_and_map_color__("Firebrick")
    except InvalidColorRepresentationError as icre:
        assert "firebrick" in icre.message


def test_draw_style_input_can_handle_enum():
    for _, enum_member in DrawStyles._value2member_map_.items():
        value = enum_member.value
        assert plot._check_draw_style(value) == enum_member.value
        assert plot._check_draw_style(enum_member) == value


def test_layer_creating_with_different_x_y_len_fails():
    x_data = np.random.randn(100)
    y_data = np.random.randn(101)
    output = plot.add_layer(
        x_data, y_data, name="test", color="firebrick", draw_style=DrawStyles.GL_POINTS
    )
    assert not output


def test_adding_several_layers_then_removing_two():
    x_data = np.random.randn(100)
    y_data = np.random.randn(100)
    layer1 = plot.add_layer(
        x_data, y_data, name="test1", color="firebrick", draw_style=DrawStyles.GL_POINTS
    )
    layer2 = plot.add_layer(
        x_data,
        y_data,
        name="test2",
        color="firebrick",
        draw_style=DrawStyles.GL_LINE_LOOP,
    )
    layer3 = plot.add_layer(
        x_data,
        y_data,
        name="test3",
        color="firebrick",
        draw_style=DrawStyles.GL_POLYGON,
    )
    layer4 = plot.add_layer(
        x_data, y_data, name="test4", color="firebrick", draw_style=DrawStyles.GL_QUADS
    )
    assert plot.remove_layer(layer3)
    assert plot.remove_layer(layer4)
    assert plot.__engine__.num_models() == 2


def test_removing_non_existant_layer_fails():
    result = plot.remove_layer(12341)
    assert not result


def test_negative_window_size_fails():
    x_data = np.random.randn(50)
    y_data = np.random.randn(50)
    timedata = np.array([range(50)])
    result = plot.add_animated_layer(
        x_data,
        y_data,
        timedata,
        color="firebrick",
        draw_style=DrawStyles.GL_POINTS,
        window_size=-5,
        name="test_layer",
    )
    assert not result


def test_window_size_gt_timedata_length_fails():
    x_data = np.random.randn(50)
    y_data = np.random.randn(50)
    timedata = np.array([range(50)])
    result = plot.add_animated_layer(
        x_data,
        y_data,
        timedata,
        color="firebrick",
        draw_style=DrawStyles.GL_POINTS,
        window_size=50,
        name="test_layer",
    )
    assert not result
