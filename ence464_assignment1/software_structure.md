# Software Structure

This is the high level documentation for the project code.


# TODO
 - (Sam/Grace - done) Integrate display code
 - (Sam/Grace - done) Add semaphore to calibration
 - (Fletcher - done) Make ADCs interrupt driven with mutex
 - (Alex) Refactor buttons code and naming conventions/style to match
 - Add priority and frequency for each task
 - (Sam - done) Simplify calibration routine
 - (Grace - no longer required) Profiling
 - (everyone) Make a start on the report

## Tasks

### Priority 5

- Button interface

### Priority 4

- Yaw control task
- Height control task

### Priority 3

- Display task

### Priority 2

- Serial debug task

### Priority 1

- Calibration task

### Priority 0 (Same as idle)

- **NOT IMPLEMENTED:** Profiling task

## Other structures

### Queues

- `serial_print`. Tasks send null terminated strings, which are received by the serial debug task.
- `display_messages`. Tasks send `Message`s, which are received by the display task.

### Mutexes

- 

### Semaphores

-  