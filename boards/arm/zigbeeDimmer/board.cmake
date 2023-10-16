# Copyright (c) 2021 Argentum Systems Ltd.
# SPDX-License-Identifier: Apache-2.0
board_runner_args(jlink "--device=ATSAMR30E18A" "--speed=4000")

include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
