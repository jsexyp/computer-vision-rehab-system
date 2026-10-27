**Real-Time Computer Vision Rehabilitation System**

A university engineering project that combines computer vision and embedded systems to provide real-time posture feedback during rehabilitation exercises.

**Overview**

The system uses a webcam and a Python application to detect body landmarks and evaluate posture during exercises focused on trunk alignment and arm positioning. The results are communicated to embedded controllers that provide immediate feedback through an LCD and a physical LED board.
The project was developed in collaboration with a university orthopedic physician to translate exercise requierements into software and hardware functionality.

**Key Features**
- Real-time pose estimation using MediaPipe Pose (BlazePose) and OpenCV.
- Body landmark tracking and joint-angle calculations.
- Rule-based posture evaluation for trunk alignment and arm positioning.
- Visual feedback and corrective instructions on the computer screen.
- USB CDC communication between Python and a PIC18F45K50 → UART communication between the PIC and an Arduino Uno.
- LCD messages and LED indicators reflect posture status.

**System Architecture**

*1. Computer vision (Python)*

    Processes webcam frames, detects body landmarks, calculates relevant angles and evaluates posture.
  
*2. PC-to-PIC communication (USB CDC)*

    Sends posture status using a structured message format.
  
*3. Embedded processsing (PIC)*

    Interprets incoming messages, updates the LCD and generates LED control data.
  
*4. PIC-to-Arduino communication (UART)*

    Transmits LED states to the Arduino
  
*5. Physical feedback (Arduino)*

    Controls the LED board according to the received status

**Repository Structure**
- python/ - Computer vision and posture evaluation
- pic/ - PIC firmware and LCD/communication logic
- arduino/ - Arduino firmware
- docs/ - Architecture diagrams and demonstration images

**Setup and Usage**

See the documentation environment setup, Python dependencies, serial communication configuration, microcontroller programming and hardware requirements. The system requires a compatible webcam and the corresponding embedded hardware for physical feedback.

**Limitations**

This is an educational engineering prototype designed to demonstrate posture feedback and software-hardware integration. **It is not a certified medical device and does not replace professional medical evaluation or rehabilitation supervision.**

**Author**

Juan Pablo Barro Arredondo | 
Mechatronics Engineering Student
