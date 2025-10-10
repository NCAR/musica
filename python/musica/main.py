import argparse
import datetime
import logging
import shutil
import os
from musica import Examples
from musica import __version__
import musica.examples
import importlib.resources as ir


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
        action="append",
        help=("Path to save the example script to.")
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
        version=f'musica {__version__}',
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


def main():
    start = datetime.datetime.now()

    args = parse_arguments()
    setup_logging(args.verbose)

    logger = logging.getLogger(__name__)

    logger.debug(f"{__file__}")
    logger.info(f"Start time: {start}")

    logger.debug(f"Working directory = {os.getcwd()}")

    example = next(e for e in Examples if e.short_name == args.example)
    if example is None:
        raise ValueError(f"Example not found: {args.example}")
    if not args.output:
        logger.debug("No output path specified, using current directory.")

    logger.info(f"Copying example: {example} to {args.output}")

    with ir.path(musica.examples, example.path) as example_path:
        logger.info(f"Copying example from {example_path} to {args.output}")
        shutil.copy(example_path, args.output[0] if args.output and len(args.output) > 0 else '.')

    end = datetime.datetime.now()
    logger.info(f"End time: {end}")
    logger.info(f"Elapsed time: {end - start} seconds")


if __name__ == "__main__":
    main()
