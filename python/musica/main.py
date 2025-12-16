import argparse
import datetime
import logging
import shutil
import os
from musica import Examples
from musica import __version__
import musica.examples
import importlib.resources as ir
import musica


def versions():
    return f"musica {musica.__version__} (MICM {musica.micm.__version__}, TUV-x {musica.tuvx.__version__}, CARMA {musica.carma.__version__})"


def format_examples_help(examples):
    return '\n'.join(f"{e.short_name}: {e.description}" for e in examples)


def parse_arguments():
    parser = argparse.ArgumentParser(
        description='musica CLI',
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        '-e', '--example',
        type=str,
        choices=[e.short_name for e in Examples],
        help=f'Name of the example to copy out.\nAvailable examples:\n{format_examples_help(Examples)}'
    )
    parser.add_argument(
        '-o', '--output',
        type=str,
        help=("Path to save the output to.")
    )
    parser.add_argument(
        '-v', '--verbose',
        action='count',
        default=0,
        help='Increase logging verbosity. Use -v for info, -vv for debug.'
    )
    parser.add_argument(
        '--version',
        action='version',
        version=versions(),
    )
    parser.add_argument(
        '--convert',
        type=str,
        help='Path to a musica v0 configuration to convert to v1 format.'
    )
    return parser.parse_args()


def setup_logging(verbosity):
    log_level = logging.DEBUG if verbosity >= 2 else logging.INFO if verbosity == 1 else logging.CRITICAL
    datefmt = '%Y-%m-%d %H:%M:%S'
    format_string = '%(asctime)s - %(levelname)s - %(module)s.%(funcName)s - %(message)s'
    formatter = logging.Formatter(format_string, datefmt=datefmt)
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    console_handler.setLevel(log_level)
    logging.basicConfig(level=log_level, handlers=[console_handler])


def copy_example(logger, example_arg, output_path):
    example = next((e for e in Examples if e.short_name == example_arg), None)
    if not example:
        raise ValueError(f"Example '{example_arg}' not found.")

    logger.info(f"Copying example: {example} to {output_path}")

    with ir.path(musica.examples, example.path) as example_path:
        logger.info(f"Copying example from {example_path} to {output_path}")
        shutil.copy(example_path, output_path)


def convert_configuration(logger, configuration, output_path):
    logger.info(f"Converting configuration: {configuration} to {output_path}")
    if not os.path.exists(configuration):
        raise ValueError(f"Configuration file '{configuration}' does not exist.")

    parser = musica.mechanism_configuration.Parser()
    mechanism = parser.parse_and_convert_v0(configuration)
    mechanism.export(output_path)


def main():
    start = datetime.datetime.now()

    args = parse_arguments()
    setup_logging(args.verbose)

    logger = logging.getLogger(__name__)

    logger.debug(f"{__file__}")
    logger.info(f"Start time: {start}")

    logger.debug(f"Working directory = {os.getcwd()}")

    # Figure out what action we are doing. We are either converting or copying an example.
    convert = args.convert
    example_arg = args.example
    output = args.output

    if not convert and not example_arg:
        raise ValueError("Either --convert or --example must be specified.")

    if convert and example_arg:
        raise ValueError("Cannot specify both --convert and --example. Choose one.")

    if not output:
        logger.debug("No output path specified, using current directory.")
        output = '.'

    if convert:
        convert_configuration(logger, convert, output)
    else:
        copy_example(logger, example_arg, output)

    end = datetime.datetime.now()
    logger.info(f"End time: {end}")
    logger.info(f"Elapsed time: {end - start} seconds")


if __name__ == "__main__":
    main()
