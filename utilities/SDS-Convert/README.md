# SDS-Convert
Convert SDS data recordings to selected CSV format, based on descriptions found in YAML files.

Note that YAML and SDS file pairs must be passed as arguments in the same order to ensure recorded data
is decoded correctly.

### [Qeexo format V2](https://docs.qeexo.com/guides/userguides/data-management#2-1-Data-format-specification)
By default interval of 50 ms is used for timestamp increments. User can override this setting by
passing number of ms after `--interval` flag. User can also define text in label column by passing a string
after `--label` flag.

## Set-up and requirements
### Requirements
- Python 3.10+ with packages:
  - pyyaml

### Set-up
1. Open terminal in SDS-Convert root folder
2. Check installed Python version with:
   ```
   python --version
   ```
3. (Optional) Use Python environment
   1. Create Python environment:
      ```
      python -m venv <env_name>
      ```
      <sup>Note: Usually **`env`** is used for `<env_name>`</sup>
   2. Activate created Python environment:
      ```
      <env_name>/Scripts/activate
      ```
4. Install required Python packages:
   ```
   pip install pyyaml
   ```

## Usage
Print help with:
```
python sds-convert.py --help
```
### Run tool
To convert data into CSV format run:
```
python sds-convert.py -y <description_filename>.yml [<description_filename2>.yml ...] -s <sds_data_filename>.sds [<sds_data_filename2>.sds ...] -o <output_filename>.csv -f <csv_format> [--label <label>] [--interval <interval_ms>]
```
