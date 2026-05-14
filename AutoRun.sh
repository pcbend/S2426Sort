#!/usr/bin/env bash

# Exit immediately on error (-e)
# Treat unset variables as errors (-u)
# Catch errors in pipelines (pipefail)
set -euo pipefail

# -------------------------------
# Configuration
# -------------------------------

# List of run numbers to process
#RUNS=(62089 62090 62092 62094 62095 62096 62097 62099 62102 62105 62274 62275 62276 62277)
RUNS=(62094 62095 62097 62099)

# Directory containing input .mid files
DATA_DIR="/data1/yzhu/Projects/S2426/raw_data"

# Path to sorting executable
SORTER="./bin/s2426Sort"

# Output directory for ROOT files
OUT_DIR="./histOutput"

# Maximum number of runs processed in parallel
MAX_PARALLEL=1

# Log file to record failed subruns
RUN_LOG="${OUT_DIR}/run.log"

# Create output directory if it does not exist
mkdir -p "${OUT_DIR}"

# Initialize (empty) log file
: > "${RUN_LOG}"

# -------------------------------
# Function: process one run
# -------------------------------
process_run() {
  local run="$1"       # Current run number
  local files=()       # Array to store subrun files

  # Enable nullglob so that no-match patterns expand to empty array
  shopt -s nullglob
  files=(${DATA_DIR}/run${run}_*.mid)
  shopt -u nullglob

  # If no files found, skip this run
  if [ ${#files[@]} -eq 0 ]; then
    echo "[WARN] No subrun files for run ${run}"
    return 0
  fi

  echo "[INFO] Run ${run} start (${#files[@]} subruns)"

  # Loop over each subrun (serial execution within one run)
  for midfile in "${files[@]}"; do
    echo "[INFO]   Processing $(basename "${midfile}")"

    # Run sorter and check if it succeeds
    if "${SORTER}" "${midfile}"; then
      echo "[INFO]   Finished $(basename "${midfile}")"
    else
      # If sorter crashes (e.g., segfault), log the file name
      echo "${midfile}" >> "${RUN_LOG}"
      echo "[ERROR]   Failed on $(basename "${midfile}")"

      # Stop processing this run (no merge)
      return 1
    fi
  done

  echo "[INFO] Run ${run} merging..."

  # Merge all subrun ROOT files into a single file
  # Note: wildcard is intentionally unquoted for expansion
  hadd -f "${OUT_DIR}/hist${run}.root" \
       ${OUT_DIR}/hist${run}_*.root

  echo "[INFO] Run ${run} done"
}

# Export function and variables for use in subprocesses (required by xargs)
export -f process_run
export DATA_DIR SORTER OUT_DIR RUN_LOG

# -------------------------------
# Parallel execution
# -------------------------------

# Run up to MAX_PARALLEL runs in parallel
# Each run is processed independently (subruns remain serial)
printf "%s\n" "${RUNS[@]}" | \
xargs -I{} -P "${MAX_PARALLEL}" bash -c 'process_run "$@"' _ {}

# -------------------------------
# Cleanup: remove empty log file
# -------------------------------

# If log file exists but is empty, delete it
if [ ! -s "${RUN_LOG}" ]; then
  rm -f "${RUN_LOG}"
  echo "[INFO] run_log is empty, removed"
fi
