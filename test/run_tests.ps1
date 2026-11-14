$ErrorActionPreference = "Stop"
$out = Join-Path $PSScriptRoot "build"
New-Item -ItemType Directory -Force -Path $out | Out-Null
$exe = Join-Path $out "unit_tests.exe"

gcc -std=gnu99 -Wall -Wextra -Wno-unused-parameter `
    -DUNITY_SUPPORT_64 -D__USE_MINGW_ANSI_STDIO=1 `
    -Itest/unity -Itest/mocks -Isrc `
    test/unity/unity.c `
    test/mocks/mock_env.c `
    src/nx_driver_rx_wifi.c `
    src/sample_network.c `
    src/compat.c `
    src/demo_printf.c `
    test/test_nx_driver_rx_wifi.c `
    test/test_app_utils.c `
    test/test_main.c `
    -o $exe

& $exe
exit $LASTEXITCODE
