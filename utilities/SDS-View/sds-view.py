# SDS-View

import argparse
import struct
import sys

import matplotlib.pyplot as plt
import numpy as np
import yaml


# Convert C style data type to Python style
def getDataType(data_type):
    match data_type:
        case "int16_t":
            return "h"
        case "uint16_t":
            return "H"
        case "int32_t":
            return "i"
        case "uint32_t":
            return "I"
        case "float":
            return "f"
        case "double":
            return "d"
        case _:
            print(f"Unknown data type: {data_type}\n")
            return None

# Open SDS data file in read mode
def openFile(file_name):
    try:
        if ".yml" in file_name:
            f = open(file_name, "r")
        else:
            f = open(file_name, "rb")

        return f
    except Exception as e:
        print(f"Error in openFile({file_name}): {e}")
        sys.exit(1)

# Close file
def closeFile(file_name):
    try:
        file_name.close()
    except Exception as e:
        print(f"Error in closeFile({file_name}): {e}")
        sys.exit(1)

# Create new figure and plot content
def plotData(all_data, data_desc, freq, title):
    dim = {}
    desc_n = 0
    desc_n_max = len(data_desc)

    # Create a new figure for each .sds file
    fig = plt.figure()
    for desc in data_desc:
        # Extract parameters from description in YAML file
        if "unit" in desc:
            unit = desc["unit"]
        else:
            unit = "raw"

        if "scale" in desc:
            scale = desc["scale"]
        else:
            scale = 1

        if "offset" in desc:
            offset = desc["offset"]
        else:
            offset = 0

        if "type" in desc:
            d_type = getDataType(desc["type"])
        else:
            sys.exit(1)

        # Calculate number of bytes needed for decoding the data in .sds file
        d_byte = struct.calcsize(d_type)
        # Disunite raw data into a list of data points according to the number of bytes needed for each data point
        tmp_data = [all_data[i:(i + d_byte)] for i in range(0, len(all_data), d_byte)]
        # Keep only every n-th data point
        tmp_data = tmp_data[desc_n::desc_n_max]
        # Decode retrieved data points
        data = struct.unpack(f"{int(len(tmp_data))}{d_type}", b''.join(tmp_data))
        # Scale and offset data points
        scaled_data = [((x * scale) + offset) for x in data]

        # Generate timestamps using number of data points and sampling frequency
        t = np.arange(0, len(data) / freq, 1 / freq)
        plt.plot(t, scaled_data, label=desc["value"])

        # Store data points in a dictionary for later use when there are 3 axes described
        if desc_n_max == 3:
            dim[desc_n] = scaled_data

        # Increment description number
        desc_n += 1

    plt.title(title)
    plt.xlabel("seconds")
    plt.ylabel(unit)
    plt.legend()

    # Create a 3D view when there are 3 axes available
    if desc_n_max == 3:
        fig3d = plt.figure()
        ax3d = fig3d.add_subplot(projection="3d")
        ax3d.plot(dim[0], dim[1], dim[2])
        ax3d.set_title(f"{title} - 3D")
        ax3d.set_xlabel(f"{data_desc[0]['value']} [{data_desc[0]['unit']}]")
        ax3d.set_ylabel(f"{data_desc[1]['value']} [{data_desc[1]['unit']}]")
        ax3d.set_zlabel(f"{data_desc[2]['value']} [{data_desc[2]['unit']}]")

# Main function
def main():
    parser = argparse.ArgumentParser(description="View SDS data")
    required = parser.add_argument_group("required")
    required.add_argument("-y", "--yml", help="YAML sensor description file", required=True)
    required.add_argument("-s", "--sds", help="SDS data recording file", nargs="+", required=True)
    args = parser.parse_args()

    # Load data from .yml file
    meta_file = openFile(args.yml)
    meta_data = yaml.load(meta_file, Loader=yaml.FullLoader)["sds"]
    closeFile(meta_file)

    # Parse description file
    data_name = meta_data["name"]
    data_desc = meta_data["content"]
    data_freq = meta_data["frequency"]
    if not data_freq > 0:
        print(f"Error: Sample frequency must be greater than 0 (f = {data_freq})\n")
        sys.exit(0)

    # Read .sds file/files
    data = {}
    for arg in args.sds:
        data_file = openFile(arg)
        data[f"{arg}"] = data_file.read()
        closeFile(data_file)

    # Plot data from .sds file/files
    for d in data:
        plotData(data[d], data_desc, data_freq, data_name)

    # Show plotted figures
    plt.grid(linestyle=":")
    plt.show()


# --- Main ---
if __name__ == "__main__":
    main()
