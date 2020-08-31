import os, sys
import warnings
# Hide deprecation warnings from numpy and tensorflow
warnings.filterwarnings('ignore',category=FutureWarning)
stderr = sys.stderr
sys.stderr = open(os.devnull, 'w')
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

import face_recognition
import cv2
import dlib
import cvlib
from mtcnn.mtcnn import MTCNN
sys.stderr = stderr

class FaceDetectors:
  def __init__(self, filename, is_known_fake):
    self.filename = filename
    self.is_known_fake = is_known_fake

  def get_results(self):
    len_faces = len(self.faces)
    # (true positive, true negative, false positive, false negative)
    if len_faces == 0:
      if self.is_known_fake:
        return [0, 1, 0, 0]
      else:
        return [0, 0, 0, 1]
    else:
      if self.is_known_fake:
        return [0, 0, len_faces, 0]
      else:
        return [1, 0, len_faces - 1, 0]

  def detectFaceViaMTCNNFaceDetector(self):
    img = cv2.imread(self.filename)
    self.faces = MTCNN().detect_faces(img)
    return self.get_results()

  def detectFaceViaHaarCascadeFaceDetector(self):
    face_cascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
    img = cv2.imread(self.filename)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    self.faces = face_cascade.detectMultiScale(gray)
    return self.get_results()

  def detectFaceViaHoGFaceDetector(self):
    hog_face_detector = dlib.get_frontal_face_detector()
    self.faces = hog_face_detector(dlib.load_rgb_image(
        self.filename), 1)  # 1 - unsampled count
    return self.get_results()

  def detectFaceViaCNNFaceDetector(self):
    cnn_face_detector = dlib.cnn_face_detection_model_v1(
        "mmod_human_face_detector.dat")
    self.faces = cnn_face_detector(dlib.load_rgb_image(
        self.filename), 1)  # 1 - unsampled count
    return self.get_results()

  def detectFaceViaDNNFaceDetector(self):
    img = cv2.imread(self.filename)
    faces, _ = cvlib.detect_face(img)
    self.faces = faces
    return self.get_results()
   
