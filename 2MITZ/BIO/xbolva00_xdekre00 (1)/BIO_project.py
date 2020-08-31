import pandas as pd
import numpy as np
import glob
import os
import sys
import argparse
import pathlib
import shutil
import face_recognition
import matplotlib
import matplotlib.pyplot as plt

from skimage import io, img_as_ubyte
from skimage.transform import resize
from PIL import Image, ImageEnhance
from Detectors import FaceDetectors


# Get all images in folder
def get_images_in_folder_recursively(img_dir):
    return list(pathlib.Path(img_dir).rglob("*.jpg")) + \
        list(pathlib.Path(img_dir).rglob("*.png"))


# Find all face images by certain face width
def filter_images_by_face_width(img_dir, face_width, img_limit, out_dir):
    images = get_images_in_folder_recursively(img_dir)
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    c = 0
    for img in images:
        face_locs = face_recognition.face_locations(
            face_recognition.load_image_file(img))
        if len(face_locs) == 1:
            current_image = face_locs[0]
            # (top, right, bottom, left)
            face_width_px = current_image[1] - current_image[3]
            if face_width_px == face_width:
                shutil.copy(img, out_dir)
                c += 1
                if c == img_limit:
                    return img_limit
    return c

# Run face detectors, collect results, show graph
def do_analysis_brightness_effect(base_dir):
    inner_dirs = [x[0] for x in os.walk(base_dir) if x[0] is not base_dir]
    r1 = []
    r2 = []
    r3 = []
    r4 = []
    r5 = []
    inner_dirs = sorted(inner_dirs)
    brightness_factors = []
    for dir in inner_dirs:
        pos = dir.find("brightness_")
        if pos > 0:
            brightness_factors.append(float(dir[dir.rfind("_") + 1:]))
        else:
            print("Unexpected dir:", dir)
            inner_dirs.remove(dir)

    print("Detected brightness factors:", str(brightness_factors))
    if len(brightness_factors) == 0:
        return

    for dir in inner_dirs:
            det1_tp = det1_tn = det1_fp = det1_fn = 0
            det2_tp = det2_tn = det2_fp = det2_fn = 0
            det3_tp = det3_tn = det3_fp = det3_fn = 0
            det4_tp = det4_tn = det4_fp = det4_fn = 0
            det5_tp = det5_tn = det5_fp = det5_fn = 0
            for file in os.listdir(dir):
                img_path = os.path.join(dir, file)
                D = FaceDetectors(img_path, "fake" in img_path)
                ret = D.detectFaceViaMTCNNFaceDetector()
                det1_tp += ret[0]
                det1_tn += ret[1]
                det1_fp += ret[2]
                det1_fn += ret[3]

                ret = D.detectFaceViaHaarCascadeFaceDetector()
                det2_tp += ret[0]
                det2_tn += ret[1]
                det2_fp += ret[2]
                det2_fn += ret[3]

                ret = D.detectFaceViaHoGFaceDetector()
                det3_tp += ret[0]
                det3_tn += ret[1]
                det3_fp += ret[2]
                det3_fn += ret[3]

                ret = D.detectFaceViaCNNFaceDetector()
                det4_tp += ret[0]
                det4_tn += ret[1]
                det4_fp += ret[2]
                det4_fn += ret[3]

                ret = D.detectFaceViaDNNFaceDetector()
                det5_tp += ret[0]
                det5_tn += ret[1]
                det5_fp += ret[2]
                det5_fn += ret[3]

            # Compute accuracy metric
            r1.append((det1_tp + det1_tn) /
                      (det1_tp + det1_tn + det1_fp + det1_fn))
            r2.append((det2_tp + det2_tn) /
                      (det2_tp + det2_tn + det2_fp + det2_fn))
            r3.append((det3_tp + det3_tn) /
                      (det3_tp + det3_tn + det3_fp + det3_fn))
            r4.append((det4_tp + det4_tn) /
                      (det4_tp + det4_tn + det4_fp + det4_fn))
            r5.append((det5_tp + det5_tn) /
                      (det5_tp + det5_tn + det5_fp + det5_fn))

    plt.bar(np.array(brightness_factors) - 0.08, r1,
            color='blue', width=0.03, label="MTCNN")
    plt.bar(np.array(brightness_factors) - 0.04, r2,
            color='green', width=0.03, label="Haar (opencv)")
    plt.bar(np.array(brightness_factors), r3,
            color='red', width=0.03, label="HoG (dlib)")
    plt.bar(np.array(brightness_factors) + 0.04, r4,
            color='black', width=0.03, label="CNN (dlib)")
    plt.bar(np.array(brightness_factors) + 0.08, r5,
            color='yellow', width=0.03, label="DNN (cvlib)")
    plt.xticks(np.array(brightness_factors))
    plt.ylabel('presnosť detekcie (accuracy)')
    plt.xlabel('faktor jasu')
    plt.title('Vplyv zmeny jasu na presnosť detekcie tváre')
    plt.legend()
    plt.show()


# Run face detectors, collect results, show graph
def do_analysis_downscale_effect(base_dir):
    inner_dirs = [x[0] for x in os.walk(base_dir) if x[0] is not base_dir]
    r1 = []
    r2 = []
    r3 = []
    r4 = []
    r5 = []
    inner_dirs = sorted(inner_dirs)
    downscale_factors = []
    for dir in inner_dirs:
        pos = dir.find("downscale_")
        if pos > 0:
            downscale_factors.append(float(dir[dir.rfind("_") + 1:]))
        else:
            print("Unexpected dir:", dir)
            inner_dirs.remove(dir)

    print("Detected downscale factors:", str(downscale_factors))
    if len(downscale_factors) == 0:
        return

    for dir in inner_dirs:
            det1_tp = det1_tn = det1_fp = det1_fn = 0
            det2_tp = det2_tn = det2_fp = det2_fn = 0
            det3_tp = det3_tn = det3_fp = det3_fn = 0
            det4_tp = det4_tn = det4_fp = det4_fn = 0
            det5_tp = det5_tn = det5_fp = det5_fn = 0
            for file in os.listdir(dir):
                img_path = os.path.join(dir, file)
                D = FaceDetectors(img_path, "fake" in img_path)
                ret = D.detectFaceViaMTCNNFaceDetector()
                det1_tp += ret[0]
                det1_tn += ret[1]
                det1_fp += ret[2]
                det1_fn += ret[3]

                ret = D.detectFaceViaHaarCascadeFaceDetector()
                det2_tp += ret[0]
                det2_tn += ret[1]
                det2_fp += ret[2]
                det2_fn += ret[3]

                ret = D.detectFaceViaHoGFaceDetector()
                det3_tp += ret[0]
                det3_tn += ret[1]
                det3_fp += ret[2]
                det3_fn += ret[3]

                ret = D.detectFaceViaCNNFaceDetector()
                det4_tp += ret[0]
                det4_tn += ret[1]
                det4_fp += ret[2]
                det4_fn += ret[3]

                ret = D.detectFaceViaDNNFaceDetector()
                det5_tp += ret[0]
                det5_tn += ret[1]
                det5_fp += ret[2]
                det5_fn += ret[3]

            # Compute accuracy metric
            r1.append((det1_tp + det1_tn) /
                      (det1_tp + det1_tn + det1_fp + det1_fn))
            r2.append((det2_tp + det2_tn) /
                      (det2_tp + det2_tn + det2_fp + det2_fn))
            r3.append((det3_tp + det3_tn) /
                      (det3_tp + det3_tn + det3_fp + det3_fn))
            r4.append((det4_tp + det4_tn) /
                      (det4_tp + det4_tn + det4_fp + det4_fn))
            r5.append((det5_tp + det5_tn) /
                      (det5_tp + det5_tn + det5_fp + det5_fn))

    plt.bar(np.array(downscale_factors) - 0.02, r1,
            color='blue', width=0.01, label="MTCNN")
    plt.bar(np.array(downscale_factors) - 0.01, r2,
            color='green', width=0.01, label="Haar (opencv)")
    plt.bar(np.array(downscale_factors), r3,
            color='red', width=0.01, label="HoG (dlib)")
    plt.bar(np.array(downscale_factors) + 0.01, r4,
            color='black', width=0.01, label="CNN (dlib)")
    plt.bar(np.array(downscale_factors) + 0.02, r5,
            color='yellow', width=0.01, label="DNN (cvlib)")
    plt.xticks(np.array(downscale_factors))
    plt.ylabel('presnosť detekcie (accuracy)')
    plt.xlabel('downscale faktor')
    plt.title('Vplyv zmeny rozlíšenia na presnosť detekcie tváre')
    plt.legend()
    plt.show()


# Downscale all face images by certain downscale factor
def downscale_images(img_dir, downscale_factor, base_out_dir):
    images = get_images_in_folder_recursively(img_dir)
    out_dir = os.path.join(base_out_dir, "downscale_" + str(downscale_factor))
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    for img in images:
        image = io.imread(img)
        new_x = (int)(image.shape[0] * downscale_factor)
        new_y = (int)(image.shape[1] * downscale_factor)
        image_downscaled = resize(image, (new_x, new_y), anti_aliasing=True)
        io.imsave(os.path.join(out_dir, os.path.basename(img)),
                  img_as_ubyte(image_downscaled))

# Change brightness of all face images by certain brightness_factor factor
def change_brightness_of_images(img_dir, brightness_factor, base_out_dir):
    images = get_images_in_folder_recursively(img_dir)
    out_dir = os.path.join(
        base_out_dir, "brightness_" + str(brightness_factor))
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    for img in images:
        im = Image.open(img)
        enhancer = ImageEnhance.Brightness(im)
        newimg = enhancer.enhance(brightness_factor)
        newimg.save(os.path.join(out_dir, os.path.basename(img)))


# Main program
def main():
    brightness_factors = [0.1, 0.3, 0.6, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
    downscale_factors = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1.0]

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', "--input-dir", help="working directory (default: CWD)",
                        default=os.getcwd(), nargs='?')
    parser.add_argument('-ffw', "--filter-face-width",
                        help="filter faces by certain face width, copy them to output dir", type=int, const=90, nargs='?')
    parser.add_argument('-ffl', "--filter-face-limit",
                        help="maximal number of faces filtered to output dir", type=int, default=100, nargs='?')
    parser.add_argument('-o', "--output-dir", help="working directory (default: CWD/out)",
                        default=os.getcwd() + "/out", nargs='?')
    parser.add_argument('-cbf', "--change-brightness-factors",
                        help="change brightness of images with custom factors", nargs='*')
    parser.add_argument(
        '-d', "--downscale", help="downscale images by downscale factor; factor must be in range (0, 1>", type=float)
    parser.add_argument('-df', "--downscale-factors",
                        help="downscale images with custom factors", nargs='*')
    parser.add_argument('-beaf', "--brightness-effect-analysis-factors",
                        help="try various face detection tools on lightened/dimmed face images and show results as a graph", action='store_true')
    parser.add_argument('-deaf', "--downscale-effect-analysis-factors",
                        help="try various face detection tools on downscaled face images and show results as a graph", action='store_true')
    args = parser.parse_args()

    if args.filter_face_width:
        count = filter_images_by_face_width(
            args.input_dir, args.filter_face_width, args.filter_face_limit, args.output_dir)
        print("Found %d images. Images were saved to: %s" %
              (count, args.output_dir))
    elif args.change_brightness_factors is not None:
        if len(args.change_brightness_factors) == 0:
            print("Using default brightness factors:", str(brightness_factors))
            for f in brightness_factors:
                change_brightness_of_images(args.input_dir, f, args.output_dir)
        else:
            for f in args.change_brightness_factors:
                change_brightness_of_images(
                    args.input_dir, float(f), args.output_dir)
        print("Images were saved to:", args.output_dir)
    elif args.downscale_factors is not None:
        if len(args.downscale_factors) == 0:
            print("Using default downscale factors:", str(downscale_factors))
            for f in downscale_factors:
                downscale_images(args.input_dir, f, args.output_dir)
        else:
            for f in args.downscale_factors:
                downscale_images(args.input_dir, float(f), args.output_dir)
        print("Images were saved to:", args.output_dir)
    elif args.brightness_effect_analysis_factors:
        do_analysis_brightness_effect(args.input_dir)
    elif args.downscale_effect_analysis_factors:
        do_analysis_downscale_effect(args.input_dir)


if __name__ == "__main__":
    main()
