<!-- pandoc -f markdown -t latex assignment_brief.md -o assignment_brief.pdf -->

ENMT301 IMU Assignment 2020
===========================

The assignment is to be done on your own.   The goal is to study the signals output by an Invensense MPU9250 6-axis IMU and 3-axis magnetometer.  The signals were recorded while the IMU was stationary (well, compared to the lab frame of reference) on a table in the Mechatronics lab.

The assignment uses a jupyter notebook.  These are becoming popular
   with engineers and scientists as a useful means of combining text,
   program code, and program output (plots, etc.) in a single file.


1. Downloading
--------------

1. Download the juptyer notebook, `assignment.ipynb`, and the data
   file, `imudata.csv` from
   https://eng-git.canterbury.ac.nz/mph/enmt301-imu-assignment-2020.
   Warning, the data file is 6.3 MB uncompressed.

Note, you can view, but not run, the notebook at https://eng-git.canterbury.ac.nz/mph/enmt301-imu-assignment-2020.


2. Running the notebook on your computer
----------------------------------------

There are two common options:

1. Install anaconda (https://www.anaconda.com) and view the notebook
running jupyter.  Throw away your Matlab, Anaconda is an open-source
packaging of common python packages for data science.

2. Install Microsoft vscode (https://code.visualstudio.com) with the
   python extension and load the notebook.  Microsoft vscode is an
   open-source IDE that you will need for ENCE461.
   
You will need to run all cells.  You should then see a number of plots.   If you have a problem, post to the ENMT301 discussion forum on Learn.  Emails will be quietly ignored.
   

3. Running the notebook on a lab computer
-----------------------------------------

Anaconda and Microsoft vscode are installed on the computers in the Mechatronics and CAE labs.


4. Remote access to lab computers
---------------------------------

1. Connect with web browser to https://gostudent.canterbury.ac.nz

2. Log in with uclive email address.

3. You should see the ITS workroom and CAE workroom.

4. Click on CAE workroom icon to log in to a CAE computer.


5. Report
---------

1. Create a document (see `report-template.pdf` as a template).

2. For each plot, describe your observations and your explanations for
   what you see.  I am expecting a paragraph for each plot.
   
3. Where applicable, compare the results with specifications in the the datasheet.

4. If you are hoping for an A grade, you should include some numerical analysis.

5. Write a small piece of python code for estimating the gyroscope drift.

6. Include a plot of your estimated gyroscope offset drift.

7. Add your code as an appendix.

8. Submit your document as a PDF to Learn.
