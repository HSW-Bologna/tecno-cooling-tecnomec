import kconfiglib
import os
import multiprocessing
from pathlib import Path
import tools.meta.csv2carray as csv2carray
from tools.meta.genkconfig import generate_sdkconfig_header


def PhonyTargets(
    target,
    action,
    depends,
    env=None,
):
    # Creates a Phony target
    if not env:
        env = DefaultEnvironment()
    t = env.Alias(target, depends, action)
    env.AlwaysBuild(t)


PROGRAM = "app"
MAIN = "main"
ASSETS = "assets"
SIMULATOR = "simulator"
COMPONENTS = "components"
FREERTOS = f"{SIMULATOR}/freertos-simulator"
CJSON = f"{SIMULATOR}/cJSON"
B64 = f"{SIMULATOR}/b64"
LVGL = f"{COMPONENTS}/lvgl"
STRING_TRANSLATIONS = f"{MAIN}/adapters/view/intl"

CFLAGS = [
    "-Wall",
    "-Wextra",
    "-g",
    "-O0",
    "-DBUILD_CONFIG_SIMULATED_APP",
    "-DBUILD_CONFIG_DISPLAY_HORIZONTAL_RESOLUTION=320",
    "-DBUILD_CONFIG_DISPLAY_VERTICAL_RESOLUTION=240",
    "-DLV_USE_SDL",
    "-DESP_PLATFORM",
    "-DLV_CONF_INCLUDE_SIMPLE",
    '-DprojCOVERAGE_TEST=0',
]
LDLIBS = ["-lSDL2", "-lpthread", "-lm"]

CPPPATH = [
    COMPONENTS, f'#{SIMULATOR}/port', f'#{MAIN}',
    f"#{MAIN}/config", f"#{SIMULATOR}", B64, CJSON, f"#{LVGL}", 
]

TRANSLATIONS = [
    {
        "generated_files": [f"{STRING_TRANSLATIONS}/AUTOGEN_FILE_strings.c", f"{STRING_TRANSLATIONS}/AUTOGEN_FILE_strings.h"],
        "input_folder": f"{ASSETS}/translations/strings",
        "output_folder": STRING_TRANSLATIONS
    },
]


def main():
    num_cpu = multiprocessing.cpu_count()
    SetOption("num_jobs", num_cpu)
    print("Running with -j {}".format(GetOption('num_jobs')))

    env_options = {
        "ENV": os.environ,
        "CC": ARGUMENTS.get("cc", "gcc"),
        "ENV": os.environ,
        "CPPPATH": CPPPATH,
        'CPPDEFINES': [],
        "CCFLAGS": CFLAGS,
        "LIBS": LDLIBS,
    }

    env = Environment(**env_options)
    env.Tool('compilation_db')

    translations = []
    for translation in TRANSLATIONS:
        translations += csv2carray.create_scons_target(env, **translation)
    env.Alias("intl", translations)

    sdkconfig = env.Command(
        f"{SIMULATOR}/sdkconfig.h",
        [str(filename) for filename in Path(
            "components").rglob("Kconfig")] + ["sdkconfig"],
        generate_sdkconfig_header)

    freertos_env = env
    (freertos, include) = SConscript(
        f'{FREERTOS}/SConscript', exports=['freertos_env'])
    env['CPPPATH'] += [include]

    pman_env = env
    (pman, include) = SConscript(
        f'{COMPONENTS}/c-page-manager/SConscript', exports=['pman_env'])
    env['CPPPATH'] += [include]

    c_debounce_env = env
    (debounce, include) = SConscript(
        f'{COMPONENTS}/c-debounce/SConscript', exports=["c_debounce_env"])
    env['CPPPATH'] += [include]

    c_watcher_env = env
    (watcher, include) = SConscript(
        f'{COMPONENTS}/c-watcher/SConscript', exports=["c_watcher_env"])
    env['CPPPATH'] += [include]

    i2c_selected = [ "rtc/RX8010", "io/MCP23008", "dummy"]
    i2c_env = env
    (i2c_drivers, include) = SConscript(
        f'{COMPONENTS}/c-i2c-drivers/SConscript', exports=['i2c_env', "i2c_selected"])
    env['CPPPATH'] += [include]

    spi_selected = [ "io/MCP23S08", "dummy"]
    spi_env = env
    (spi_drivers, include) = SConscript(
        f'{COMPONENTS}/c-spi-drivers/SConscript', exports=['spi_env', "spi_selected"])
    env['CPPPATH'] += [include]

    sources = Glob(f'{SIMULATOR}/*.c')
    sources += Glob(f'{SIMULATOR}/port/*.c')
    sources += [File(filename) for filename in Path('main/model').rglob('*.c')]
    sources += [File(filename)
                for filename in Path('main/config').rglob('*.c')]
    sources += [File(filename) for filename in Path('main/adapters').rglob('*.c')]
    sources += [File(filename)
                for filename in Path('main/controller').glob('*.c')]
    sources += [File(filename)
                for filename in Path('main/services').rglob('*.c')]
    sources += [File(filename)
                for filename in Path(f'{LVGL}/src').rglob('*.c')]
    sources += [File(f'{CJSON}/cJSON.c')]
    sources += [File(f'{B64}/encode.c'),
                File(f'{B64}/decode.c'), File(f'{B64}/buffer.c')]

    prog = env.Program(PROGRAM, sdkconfig + sources + freertos + pman + debounce + i2c_drivers + spi_drivers + watcher)
    env.Depends(prog, translations)
    PhonyTargets("run", f"./{PROGRAM}", prog, env)
    compileDB = env.CompilationDatabase('build/compile_commands.json')
    env.Depends(prog, compileDB)


main()
