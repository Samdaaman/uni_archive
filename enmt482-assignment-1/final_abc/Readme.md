# Part A
 - **[calibration_fitting:](part_a/calibration_fitting.py)** A script used to fit various sensor models that are an improvement on
the classic "inverse IR model".
 - **[calibration_manual_tweaks.xlsx](part_a/calibration_manual_tweaks.xlsx)** Spreadsheet where calibration tweaks were made (exported from google docs)
 - **[calibration_models.py:](part_a/calibration_models.py)** Library of unfitted models used by calibration_fitting.py
 - **[calibration_results:](part_a/calibration_results.py)** Plotting the results of calibration
 - **[extended_kalman_filter:](part_a/extended_kalman_filter.py)** EKF implementation and plotting
 - **[models:](part_a/models.py)** Classes for creating fitted models

# Part B
 - **[demo:](part_b/demo.py)** Module used to run the particle filter.
 - **[models:](part_b/models.py)** Model methods for the odometry motion model, relative sensor model and absolute sensor model.
 - **[plot:](part_b/plot.py)** Methods for plotting beacons and routes. Custom plot functions added for variable particle renderings and beacon_guess positions.
 - **[utils:](part_b/utils.py)** Methods for resampling and statistical processing. 
