import pandas as pd
import sys
import struct
from pathlib import Path


def generate_opencl_friendly_prob_grid(gsd_prob_grid_path: Path, subsample_size: int, should_correct=False,
                                       valid_psi_rho_pairs=None):
    """
    Generates a probability grid in the format understandable for the OpenCL-accelerated code. Furthermore, it takes
    into account whether the probability grid should should use the estimation correction (for the range of rho and
    psi).

    :param gsd_prob_grid_path: filepath of the pickle file with GSD's probability grid
    :param subsample_size: the number of observations in each bootstrap sample
    :param should_correct: a flag indicating whether to use the estimation correction
    :param valid_psi_rho_pairs: pandas DataFrame indexed with (psi, rho) pairs. If for a given pair the value is 0, it
     should be excluded from the estimation process. If it is 1, it should be included.
    :return: filename of the binary file generated (the one with the probability grid)
    """
    if not should_correct:
        output_filename = gsd_prob_grid_path.stem + ".bin"
    else:
        # Append to the filename the info about subsample size for which the probability grid was generated
        output_filename = "_".join([gsd_prob_grid_path.stem, "n" + str(subsample_size)]) + ".bin"

    # Do not generate a new binary file if the one for the current subsample size exists
    if Path(output_filename).exists():
        print(f"The {output_filename} file already exists! Skipping its generation.")
        return output_filename

    full_prob_grid = pd.read_pickle(gsd_prob_grid_path)
    out_prob_grid = []

    # Generate a probability grid understandable for the OpenCL-accelerated code
    # probs --- probabilities
    if not should_correct:
        for (_psi, _rho), probs in full_prob_grid.iterrows():
            out_prob_grid.append(_psi)
            out_prob_grid.append(_rho)
            for _idx in range(1, 6):
                out_prob_grid.append(probs[_idx])
    else:
        for ((_psi, _rho), probs), ((__psi, __rho), is_valid) in \
                zip(full_prob_grid.iterrows(), valid_psi_rho_pairs.iterrows()):
            assert _psi == __psi and _rho == __rho
            assert len(is_valid) == 1
            if not is_valid.all():  # using iloc[0] to convert series into a scalar
                continue
            out_prob_grid.append(_psi)
            out_prob_grid.append(_rho)
            for _idx in range(1, 6):
                out_prob_grid.append(probs[_idx])

    # Store the output probability grid in a binary file
    with open(output_filename, "wb") as _out_file:
        _out_file.write(struct.pack('d' * len(out_prob_grid), *out_prob_grid))

    return output_filename


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("ERROR")
        sys.exit(1)

    filename = sys.argv[1]

    try:
        dataframe = pd.read_pickle(filename)
    except IOError:
        print("File does not exists")
        sys.exit(1)

    print(dataframe)
    out_array = []

    for (psi, ro), row in dataframe.iterrows():
        out_array.append(psi)
        out_array.append(ro)
        for idx in range(1, 6):
            out_array.append(row[idx])

    out_file = open(filename.split(".")[0] + ".bin", "wb")
    wrote = out_file.write(struct.pack('d'*len(out_array), *out_array))
    out_file.close()

    exit(0)
