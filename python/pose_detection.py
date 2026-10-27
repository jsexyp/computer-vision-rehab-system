# -*- coding: utf-8 -*-
"""
Project: Real-Time Rehab System
Purpose:
    Detect body landmarks from webcams, evaluate posture,
    and transmit posture status to a PIC microcontroller for embedded feedback.
Technologies:
    Python, OpenCV, MediaPipe Pose, pySerial
Communication:
    USB Serial, PIC Microcontroller

Note: Educational prototype, not intended for clinical diagnosis.
"""

import cv2
import mediapipe as mp
import numpy as np
import serial
import time

# =========================================================
# CONFIGURACION CAMARA
# =========================================================

width = 640
height = 480

capture = cv2.VideoCapture(0, cv2.CAP_DSHOW)

capture.set(cv2.CAP_PROP_FRAME_WIDTH, width)
capture.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

# =========================================================
# CONEXION PIC
# =========================================================

pic_port = 'COM7' #Update the COM port to match your PIC

pic = serial.Serial(pic_port, 9600)

time.sleep(2)

print(f"PIC conectado en: {pic_port}")

# =========================================================
# MEDIAPIPE
# =========================================================

mp_pose = mp.solutions.pose
mp_drawing = mp.solutions.drawing_utils

pose = mp_pose.Pose(
    static_image_mode=False,
    model_complexity=1,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5
)

# =========================================================
# FUNCION ANGULO
# =========================================================

def angulo(p1, p2):

    p1 = np.array(p1)
    p2 = np.array(p2)

    v = p2 - p1

    vertical = np.array([0, -1])

    cos_ang = np.dot(v, vertical) / (
        np.linalg.norm(v) * np.linalg.norm(vertical)
    )

    cos_ang = np.clip(cos_ang, -1.0, 1.0)

    ang = np.degrees(np.arccos(cos_ang))

    return ang

# =========================================================
# VARIABLE CONTROL SERIAL
# =========================================================

mensaje_anterior = ""

print("Sistema iniciado")

# =========================================================
# LOOP PRINCIPAL
# =========================================================

while True:

    ret, frame = capture.read()

    if not ret:
        break

    frameRGB = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = pose.process(frameRGB)

    if results.pose_landmarks:

        mp_drawing.draw_landmarks(
            frame,
            results.pose_landmarks,
            mp_pose.POSE_CONNECTIONS
        )

        landmarks = results.pose_landmarks.landmark

        # =================================================
        # PUNTOS
        # =================================================

        oreja = [
            landmarks[7].x * width,
            landmarks[7].y * height
        ]

        hombro = [
            landmarks[11].x * width,
            landmarks[11].y * height
        ]

        cadera = [
            landmarks[23].x * width,
            landmarks[23].y * height
        ]

        codo = [
            landmarks[13].x * width,
            landmarks[13].y * height
        ]

        # =================================================
        # ANGULOS
        # =================================================

        oh = 180 - angulo(oreja, hombro)

        hc = 180 - angulo(hombro, cadera)

        columna = 180 - angulo(oreja, cadera)

        post12 = angulo(hombro, codo)

        # =================================================
        # DETERMINAR POSICION
        # =================================================

        posicion = 0

        if 87 < post12 < 96:

            posicion = 1

            mensaje_pos = "POSICION 1"

        elif 1 < post12 < 6:

            posicion = 2

            mensaje_pos = "POSICION 2"

        elif 175 < post12 < 182:

            posicion = 3

            mensaje_pos = "POSICION 3"

        else:

            mensaje_pos = "BRAZO INCORRECTO"

        # =================================================
        # VALIDACIONES
        # =================================================

        cabeza_ok = 1 if columna < 12 else 0

        hombro_ok = 1 if oh < 40 else 0

        codo_ok = 1 if hc < 8 else 0

        brazo_ok = 1 if posicion != 0 else 0

        # =================================================
        # MENSAJE SERIAL
        # =================================================

        mensaje_serial = (
            f"P{posicion},"
            f"H{cabeza_ok},"
            f"S{hombro_ok},"
            f"C{codo_ok},"
            f"B{brazo_ok}\n"
        )

        # =================================================
        # ENVIAR SOLO SI CAMBIA
        # =================================================

        if mensaje_serial != mensaje_anterior:

            pic.write(mensaje_serial.encode())

            print("Enviado:", mensaje_serial)

            mensaje_anterior = mensaje_serial

        # =================================================
        # VISUALIZACION
        # =================================================

        color = (0,255,0)

        if cabeza_ok and hombro_ok and codo_ok:

            mensaje = "POSTURA CORRECTA"

        else:

            mensaje = "CORRIJA POSTURA"

            color = (0,0,255)

        cv2.putText(
            frame,
            mensaje,
            (10,40),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.8,
            color,
            2
        )

        cv2.putText(
            frame,
            mensaje_pos,
            (10,70),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.8,
            color,
            2
        )

        cv2.putText(
            frame,
            f"Columna: {columna:.1f}",
            (10,110),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.6,
            (255,255,255),
            1
        )

        cv2.putText(
            frame,
            f"OH: {oh:.1f}",
            (10,140),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.6,
            (255,255,255),
            1
        )

        cv2.putText(
            frame,
            f"HC: {hc:.1f}",
            (10,170),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.6,
            (255,255,255),
            1
        )

        cv2.putText(
            frame,
            f"Brazo: {post12:.1f}",
            (10,200),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.6,
            (255,255,255),
            1
        )

    cv2.imshow("Rehabilitacion", frame)

    if cv2.waitKey(5) & 0xFF == ord('q'):
        break

# =========================================================
# CERRAR
# =========================================================

pic.close()

capture.release()

cv2.destroyAllWindows()
