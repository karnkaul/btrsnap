from dataclasses import dataclass
import argparse
import os
import subprocess


def panic(message: str, exit_code=1):
  print(f"PANIC: {message}")
  exit(exit_code)


@dataclass
class Params:
  build_preset: str
  build_dir: str
  build_config: str
  output_dir: str


class Installer:
  m_params: Params

  def __init__(self, params: Params):
    self.m_params = params

  def configure(self: Installer):
    cmd = [
      "cmake",
      "-S=.",
      f"-B={self.m_params.build_dir}",
      f"--preset={self.m_params.build_preset}",
    ]
    self._execute(command=cmd, error="failed to configure")

  def build(self: Installer):
    cmd = [
      "cmake",
      "--build",
      self.m_params.build_dir,
      f"--config={self.m_params.build_config}",
    ]
    self._execute(command=cmd, error="failed to build")

  def install(self: Installer):
    print(f"installing to: {self.m_params.output_dir}")
    cmd = [
      "cmake",
      "--install",
      self.m_params.build_dir,
      f"--config={self.m_params.build_config}",
      f"--prefix={self.m_params.output_dir}",
    ]
    self._execute(command=cmd, error="failed to install")

  def _execute(self, command: list[str], error: str):
    result = subprocess.run(command)
    if result.returncode != 0:
      panic(error)


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser()
  parser.add_argument("output", help="output directory")
  parser.add_argument("-p", "--preset", default="default", help="configure preset to use (default)")
  parser.add_argument("-c", "--config", default="RelWithDebInfo", help="build config to use (RelWithDebInfo)")
  parser.add_argument("-b", "--build", default="out/install", help="build directory (out/install)")
  return parser.parse_args()


def verify_pwd():
  script_dir = os.path.dirname(os.path.realpath(__file__))
  project_dir = os.path.dirname(script_dir)
  if os.path.realpath(os.curdir) != project_dir:
    panic(f"run script from project root ({project_dir})")


def run_installer(params: Params):
  installer = Installer(params=params)
  installer.configure()
  installer.build()
  installer.install()


def main():
  args = parse_args()
  verify_pwd()
  params = Params(
    build_preset=args.preset,
    build_dir=args.build,
    build_config=args.config,
    output_dir=args.output,
  )
  run_installer(params)


if __name__ == "__main__":
  main()
