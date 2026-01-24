import subprocess
import sys
import platform
import argparse
from pathlib import Path


OS_NAME = platform.system().lower()

def run_command(command):
    """
    运行系统命令。
    check=True: 如果命令返回非零状态码，直接抛出 CalledProcessError 异常。
    """
    print(f"\n[RUNNING] {' '.join(command)}")
    # 移除手动 returncode 判断，交给 check=True 处理
    subprocess.run(command, check=True)

def get_compiler_configs(test_mode):
    """
    根据操作系统返回对应的编译器配置
    字段规则:
    (test name, c compiler, c++ compiler, build dir)
    """
    
    if OS_NAME == "windows":
        print("--- Windows Environment Detected ---")
        all_configs = [
            ("MSVC",   "cl",                     "cl",                     "msvc"),
            ("MinGW",  "x86_64-w64-mingw32-gcc", "x86_64-w64-mingw32-g++", "mingw"),
            ("Clang",  "clang",                  "clang++",                "clang"),
        ]
    elif OS_NAME == "linux":
        print("--- Linux Environment Detected ---")
        all_configs = [
            ("Clang-17", "clang-17", "clang++-17", "clang17"),
            ("GCC-13", "gcc-13", "g++-13", "gcc13")
        ]
    else:
        raise RuntimeError(f"Unsupported platform: {OS_NAME}.")
    
    # 如果是 min 模式，只取第一个编译器
    return all_configs[:1] if test_mode == "min" else all_configs

def main():
    try:
        # 命令行参数解析
        parser = argparse.ArgumentParser(description="build and test.")
        parser.add_argument(
            "--test_mode",
            choices=["min", "max"],
            default="min",
            help="Build mode: 'min' for single compiler only, 'max' for all compilers."
        )
        args = parser.parse_args()

        test_mode = args.test_mode


        # resolve() 获取绝对路径，不受当前终端工作目录影响
        script_dir = Path(__file__).parent.resolve()
        project_root = script_dir.parent
        build_base = project_root / "build"

        # 获取当前平台的编译器列表
        configs = get_compiler_configs(test_mode)

        for name, c_compiler, cxx_compiler, build_subdir in configs:
            current_build_dir = build_base / (OS_NAME + '_' + build_subdir)

            # 1. 配置
            run_command([
                "cmake",
                "-S", str(project_root),
                "-B", str(current_build_dir),
                "-G", "Ninja Multi-Config",
                f"-DCMAKE_C_COMPILER={c_compiler}",
                f"-DCMAKE_CXX_COMPILER={cxx_compiler}",
                "-DINFRA_BUILD_TESTS=ON"
            ])

            print("\n" + "=" * 50)
            print(f" Test Target: {name}")
            print("=" * 50)

            build_configs = []
            if test_mode == 'max':
                build_configs = ['Debug', 'Release']
            elif test_mode == 'min':
                build_configs = ['Release']

            # multt-config
            for build_cfg in build_configs:
                # 2. 编译
                run_command([
                    "cmake",
                    "--build", str(current_build_dir),
                    "--config", str(build_cfg)
                ])

                # 3. 测试
                run_command([
                    "ctest", "--output-on-failure",
                    "--test-dir", str(current_build_dir),
                    "-C", str(build_cfg)
                ])

        # --- 所有流程结束后的成功打印 ---
        final_msg = f" ALL BUILDS AND TESTS PASSED (test mode: {test_mode}) "
        print(f"\n{'#' * 60}")
        print(f"{final_msg:^60}")
        print(f"{'#' * 60}\n")

    except subprocess.CalledProcessError as e:
        # 当任意一个 cmake/ctest 命令返回非 0 值时触发
        print(f"\n{'#' * 60}")
        print(f"\n[ERROR] Command failed in target '{name}'")
        print(f"Exit Code: {e.returncode}")
        print(f"\n{'#' * 60}")
        sys.exit(1)
    except Exception as e:
        # 处理其他意外异常（如路径权限、编译器不存在等）
        print(f"\n{'#' * 60}")
        print(f"\n[FATAL ERROR] {str(e)}")
        print(f"\n{'#' * 60}")
        sys.exit(1)

if __name__ == "__main__":
    main()