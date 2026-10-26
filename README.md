# snsr288x -- C driver for SNSR288x series devices

## Setup

Clone this repo:
```
git clone https://github.com/phreaknik/snsr288x.git
```
Include the source files in your C project. Create a `snsr288x_config.h` for
your project, using
[snsr288x_config_template.h](inc/snsr288x_config_template.h) as a reference.

## Development

This repo uses a .clang-format style guide. To easily format your code,
install the pre-commit hook:
```
./git-pre-commit-format install
```
> This requires `clang-format` to be installed on your system. Be sure to
> download and install it. It is available for all major operating systems.

## License

This project is licensed under the [GNU Affero General Public License v3.0 or later](LICENSE) (`AGPL-3.0-or-later`).
