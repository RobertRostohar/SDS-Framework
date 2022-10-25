# SDS-View
View SDS data recording, according to the description found in a YAML file. If there are 3 axes described in the YAML file, an additional figure with a 3D view will be displayed. Otherwise all data from a single recording file will be plotted on the same figure.<br>
The tool also supports plotting of multiple recordings at the same time, by listing their paths after the `-s` flag. Note that in this case all recordings will be processed and decoded based on the description in the YAML file listed after the `-y` flag.

## Limitations
- Data in recording must all be of the same type (float, uint32, uint16, ...)
- Only one YAML file can be processed on each script run

## Set-up and requirements
### Requirements:
- SDS data recording file (*.sds*)
- SDS sensor description file (*.yml*)
- Python 3.10+
  - [requirements.txt](./script/requirements.txt)

### Set-up:
1. Open terminal in project root folder.
1. Check installed Python version with:
   ```
   python --version
   ```
1. (Optional) Create Python environment:
   ```
   python -m venv <env_name>
   ```
   <sup>Note: Usually **`env`** is used for `<env_name>`</sup>
1. (Optional) Activate created Python environment
   ```
   <env_name>/Scripts/activate
   ```
1. Install required Python packages:
   ```
   pip install -r requirements.txt
   ```

## Usage
Print help with:
```
python sds-view.py --help
```
### Run tool
To plot SDS data on run:
```
python sds-view.py -y path/to/<description_filename>.yml -s path/to/<sds_data_filename>.sds
```
#### Example:
```
python sds-view.py -y test/sensorGyro.sds.yml -s test/sensorGyro.sds
```
