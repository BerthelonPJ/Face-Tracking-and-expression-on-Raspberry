/*
 *
 *  Created by Pierre-Jean Berthelon & Florian Bucheron - 2019
 *
 * Ce projet a pour but d'utiliser la raspicam et les librairies openCV pour détecter et suivre un visage sur un flux vidéo.
 * Une fois un visage détecté, on s'assure de le suivre, c'est à dire s'assurer que le visage est bien centré sur l'écran.
 * sur ce visage sera aussi effectué une reconnaissance de sourire et des yeux.
 * Les parties du visages ainsi reconnues seront utilisées pour afficher un smiley correspondant sur le panneau led du module SenseHat de la raspi.
 */
#ifndef PROJETSY25MAIN_H
#define PROJETSY25MAIN_H

#include "ui_projetsy25main.h"
#include <raspicam/raspicam_cv.h>
#include <QTimer>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QtGlobal>
#include <QTimer>
#include <iostream>
#include <stdio.h>
#include <cv.h>
#include "SenseHat.h"

#include <QtSerialPort/QSerialPort>


using namespace cv;
using namespace std;

class ProjetSY25main : public QMainWindow, private Ui::ProjetSY25main
{
    Q_OBJECT

public:
    explicit ProjetSY25main(QWidget *parent = 0);

    /*
     * Fonction qui configure la raspicam au lancement de l'application
     */
    void configureCamera();

    /*
     * Fonction utilisée pour initialiser la liaison série avec l'arduino qui controle les moteurs.
    */
    void initPort();

    /*
     * Fonction qui transmet les commandes via la liaison série à la carte arduino.
     */
    void transmitCmd(const char*);

    /*
     * Fonction de détection de visage.
     * Prend en entrée l'image à analyser, et en sortie renvoie l'image avec les rectangles tracés autour des visages détectés.
     *
     */
    Mat detectFace(Mat);

    /*
     * Fonction de détection de sourire sur le visage détecté
     * prend en entrée la zone image correspondant au visage détecté à analyser
     * et retourne un boolean ( true si sourire détecté, false sinon)
     */
    bool detectSmile(Mat);
    /*
     * Fonction de détection de l'oeil droit sur le visage détecté
     * prend en entrée la zone image correspondant à la moitié droite du visage à analyser
     * et retourne un boolean ( true si oeil détecté, false sinon)
     */
    bool detectRightEye(Mat);
    /*
     * Fonction de détection de sourire sur le visage détecté
     * prend en entrée la zone image correspondant à la moitié gauche du visage à analyser
     * et retourne un boolean ( true si oeil détecté, false sinon)
     */
    bool detectLeftEye(Mat);

    /*
     * Fonction ayant pour but de gérer et transmettre les commandes aux servomoteurs.
     * Elle s'occupe du centrage de l'image sur le visage.
     * les deux entrées sont les positions en x et y du centre du visage.
     */
    void handleServo(int, int);

    /*
     * Fonction qui affiche l'expression de visage détectée sur le SenseHat (panneau led)
     * Elle prend en entrée trois booléen, correspondant à la détection ou non de sourire, oeil gauche et oeil droit.
     *
     */
    void displaySmiley(bool smile, bool leftEye, bool rightEye);


private slots:

    /*
     *Fonction qui s'occupe de capturer une image et de l'afficher dans le label associé.
     */
    void capturePicture();

    /*
     * appelé lors de click sur le bouton takePic
     */
    void on_takepicBtn_clicked();
    /*
     * appelé lors de click sur le bouton exit
     * Il vient quitter l'application.
     */
    void on_exitBtn_clicked();


    /*
     *Fonction qui convertit une Matrice openCV en QImage pouvant être affichée sur une interface.
     */
    inline QImage cvMatToQImage(const cv::Mat &inMat);

    void on_videoBtn_clicked();

private:

    // port série utilisé pour communiquer avec la carte arduino qui controle les moteurs.
    QSerialPort port;

    // Objet caméra pour utiliser la raspicam.
    raspicam::RaspiCam_Cv camera;
    // Image renvoyée par la camera.
    cv::Mat image;
    // image affichée dans l'interface de l'application.
    QImage image2;
    // Timer utilisé pour la capture vidéo (intervalle entre chque prise d'image)
    QTimer* timer;
    //
    Mat frame;
    // SenseHat est utilisé pour afficher les smileys sur le panneau de leds.
    SenseHat carte;
    // Compose chaque pixel du panneau led.
    uint16_t pixel;

    // La base de données de la reconnaissance de visage
    CascadeClassifier face_cascade;
    // La base de données de la reconnaissance de sourire
    CascadeClassifier smile_cascade;
    // La base de données de la reconnaissance d'oeil gauche
    CascadeClassifier left_eye_cascade;
    // La base de données de la reconnaissance d'oeil droit
    CascadeClassifier right_eye_cascade;

    // path jusqu'aux bases de données associées.
    String face_cascade_path = "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml";
    //String face_cascade_path = "/usr/share/opencv/haarcascades/haarcascade_mcs_upperbody.xml";
    String smile_cascade_path = "/usr/share/opencv/haarcascades/haarcascade_smile.xml";
    String right_eye_cascade_path = "/usr/share/opencv/haarcascades/haarcascade_righteye_2splits.xml";
    String left_eye_cascade_path = "/usr/share/opencv/haarcascades/haarcascade_lefteye_2splits.xml";

    // Rectangle de visage calculé ( ) qui définit la taille du visage détecté.
    int plusGrandRectangle=0;
    // Deux indices utilisés pour pointer vers le plus grand visage détecté dans le champ de la caméra.
    int indicePlusGrand =0;

    // définit le centre du visage détecté ( pour le suivi de visage)
    int faceCenterX;
    int faceCenterY;
};

#endif // PROJETSY25MAIN_H
