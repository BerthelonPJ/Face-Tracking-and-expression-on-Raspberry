/*
 *
 *  Created by Pierre-Jean Berthelon & Florian Bucheron - 2019
 *
 * Ce projet a pour but d'utiliser la raspicam et les librairies openCV pour détecter et suivre un visage sur un flux vidéo.
 * Une fois un visage détecté, on s'assure de le suivre, c'est à dire s'assurer que le visage est bien centré sur l'écran.
 * sur ce visage sera aussi effectué une reconnaissance de sourire et des yeux.
 * Les parties du visages ainsi reconnues seront utilisées pour afficher un smiley correspondant sur le panneau led du module SenseHat de la raspi.
 */
#include "projetsy25main.h"
#include "SenseHat.h" // pour utiliser le panneau led.


#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include <QMessageBox>
#include <QThread>

ProjetSY25main::ProjetSY25main(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this); // Initialisation de l'interface graphique.

    configureCamera();

    // chargement des bases de données à l'aide de leur path respectifs.
    face_cascade.load(face_cascade_path);
    smile_cascade.load(smile_cascade_path);
    left_eye_cascade.load(left_eye_cascade_path);
    right_eye_cascade.load(right_eye_cascade_path);


}
/*
 * Fonction qui configure la raspicam au lancement de l'application
 */
void ProjetSY25main::configureCamera(){

    camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 ); // format de la prise de photo ( ici en noir et blanc ).
    camera.set(CV_CAP_PROP_FRAME_WIDTH, 640); // format VGA : largeur de 640 pixels
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480); // format VGA : hauteur de 480 pixels
}
/*
 * Fonction utilisée pour initialiser la liaison série avec l'arduino qui controle les moteurs.
*/
void ProjetSY25main::initPort()
{
    port.setPortName("/dev/ttyACM0"); // peut varier en fonction de la raspi, à vérifier dans un terminal avec cd /dev|ls
    port.setBaudRate(QSerialPort::Baud115200); // On set le baud rate de la liaison à 115200 bauds (équivalent à celle set sur la carte arduino).


    if (!port.open(QIODevice::ReadWrite)){ // On affiche un message d'erreurs si on arrive pas à ouvrir le port en mode Read/Write.
        QMessageBox::warning(this, "impossible d'ouvrir le port", port.portName());
        return;
    }
    qDebug() <<"port opened";
}

/*
 * Fonction qui transmet les commandes via la liaison série à la carte arduino.
 */
void ProjetSY25main::transmitCmd(const char* valeur){

    // à revoir, genre faire un tableau et envoyer un byte correspondant, serait plus propre...


    qDebug() <<"transmitCmd";
    if(port.isOpen()){
        port.write(valeur);
        qDebug() <<"byte(s) written ";
        port.flush();
    }
}

/*
 *Fonction qui convertit une Matrice openCV en QImage pouvant être affichée sur une interface.
 */
inline QImage ProjetSY25main::cvMatToQImage(const cv::Mat &inMat)
   {
      switch ( inMat.type() )
      {
         // 8-bit, 4 channel
         case CV_8UC4:
         {
            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_ARGB32 );

            return image;
         }

         // 8-bit, 3 channel
         case CV_8UC3:
         {
            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_RGB888 );

            return image.rgbSwapped();
         }

         // 8-bit, 1 channel
          // C'est celui q'on utilisera car notre image est prise en noir et blanc ( en 8UC1)
         case CV_8UC1:
         {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_Grayscale8 );
#else
            static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if ( sColorTable.isEmpty() )
            {
               sColorTable.resize( 256 );

               for ( int i = 0; i < 256; ++i )
               {
                  sColorTable[i] = qRgb( i, i, i );
               }
            }

            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_Indexed8 );

            image.setColorTable( sColorTable );
#endif

            return image;
         }

         default:
            qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
      }
      return QImage();
   }

/*
 * Fonction qui affiche l'expression de visage détectée sur le SenseHat (panneau led)
 * Elle prend en entrée trois booléen, correspondant à la détection ou non de sourire, oeil gauche et oeil droit.
 *
 */
void ProjetSY25main::displaySmiley(bool smile, bool leftEye, bool rightEye){

    //visage sourire yeux ouverts
    uint8_t faceSELER[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };
    //visage sans sourire yeux ouverts
    uint8_t faceELER[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };
    //visage sourire clin d'oeil droit
    uint8_t faceSEL[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };
    //visage sourire clin d'oeil gauche
    uint8_t faceSER[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };
    //visage sans sourire clin d'oeil droit
    uint8_t faceEL[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0, 0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };
    //visage sans sourire clin d'oeil gauche
    uint8_t faceER[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };
    //visage avec sourire sans yeux
    uint8_t faceS[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };
    //visage sans sourire sans yeux
    uint8_t face[8][8][3] = {
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
        {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
    };

    if (smile & rightEye & leftEye){
        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(faceSELER[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    } else if (smile & !rightEye & leftEye){
        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(faceSEL[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    } else if (smile & rightEye & !leftEye){
        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(faceSER[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    }else if (smile & !rightEye & !leftEye){
        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(faceS[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    }else if (!smile & rightEye & leftEye){
    for(int k=0; k<8; k++){
            for(int l=1; l<9; l++){
                pixel = carte.ConvertirRGB565(faceELER[k][l-1]);
                carte.AllumerPixel(k,l,pixel);
            }
     }
    }else if (!smile & !rightEye & leftEye){
        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(faceEL[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    }else if (!smile & rightEye & !leftEye){
        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(faceER[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    }else if (!smile & !rightEye & !leftEye){
        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(face[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    }

}

/*
 * Fonction de détection de sourire sur le visage détecté
 * prend en entrée la zone image correspondant au visage détecté à analyser
 * et retourne un boolean ( true si sourire détecté, false sinon)
 */
bool ProjetSY25main::detectSmile(Mat frame){
    vector<Rect> smiles; // contiendra tous les sourires détectés

    smile_cascade.detectMultiScale(frame, smiles, 1.8,20); // fonction qui détecte les sourires dans la zone qu'on lui a donné.

    if (smiles.size() > 0){ // Si des sourires ont été détectés on return true.
       return true;
    } else { // si aucun sourire n'est détecté on return false.
        return false;
    }
}
/*
 * Fonction de détection de l'oeil droit sur le visage détecté
 * prend en entrée la zone image correspondant à la moitié droite du visage à analyser
 * et retourne un boolean ( true si oeil détecté, false sinon)
 */
bool ProjetSY25main::detectRightEye(Mat frame){
    vector<Rect> rightEye; // contiendra tous les yeux détectés

    right_eye_cascade.detectMultiScale(frame, rightEye, 1.1,1, 0); // fonction qui détecte les yeux dans la zone qu'on lui a donné.

    if (rightEye.size() > 0){ // Si des yeux ont été détectés on return true.
       return true;
    } else { // si aucun oeil n'est détecté on return false.
        return false;
    }
}
/*
 * Fonction de détection de l'oeil gauche sur le visage détecté
 * prend en entrée la zone image correspondant à la moitié gauche du visage à analyser
 * et retourne un boolean ( true si oeil détecté, false sinon)
 */
bool ProjetSY25main::detectLeftEye(Mat frame){
    vector<Rect> leftEye; // contiendra tous les yeux détectés

    left_eye_cascade.detectMultiScale(frame, leftEye, 1.1,1, 0);// fonction qui détecte les yeux dans la zone qu'on lui a donné.

    if (leftEye.size() > 0){ // Si des yeux ont été détectés on return true.
       return true;
    } else {// si aucun oeil n'est détecté on return false.
        return false;
    }
}

/*
 * Fonction ayant pour but de gérer et transmettre les commandes aux servomoteurs.
 * Elle s'occupe du centrage de l'image sur le visage.
 * les deux entrées sont les positions en x et y du centre du visage.
 */
void ProjetSY25main::handleServo(int faceCenterX, int faceCenterY){

    // gestion de l'alignement horizontal.
    // tolérance de ± 20 px pour le centre

    if(faceCenterX<300){
        transmitCmd("H");
    }
    else if(faceCenterX>340){
        transmitCmd("h");
    }
    // gestion de l'alignement vertical.
    // tolérance de ± 20 px pour le centre
    if(faceCenterY<220){
        transmitCmd("V");
    }
    else if(faceCenterY>260){
        transmitCmd("v");
    }
}


/*
 * Fonction de détection de visage.
 * Prend en entrée l'image à analyser, et en sortie renvoie l'image avec les rectangles tracés autour des visages détectés.
 *
 */

Mat ProjetSY25main::detectFace(Mat frame){

    vector<Rect> faces;
    face_cascade.detectMultiScale(frame,faces,1.1,2,0 | CV_HAAR_SCALE_IMAGE,Size(90,90)); // detection de visages sur l'image (de taille minimum 90 px * 90 px)


    if (faces.size()>0){ // Si au moins un visage est détecté.

        plusGrandRectangle=0;
        indicePlusGrand =0;

        for(size_t i=0;i<faces.size();i++) // Boucle qui vient chercher le plus grand des visages détectés.
        {
            if (faces[i].width*faces[i].height>plusGrandRectangle){ // Si le rectangle ainsi calculé est le plus grand.
                plusGrandRectangle = faces[i].width*faces[i].height; // On calcule la taille du rectangle
                indicePlusGrand = i;
            }
        }

        Mat zone = frame(faces[indicePlusGrand]); // On créé une image de taille du visage détecté (pour que les détections de sourire et d'yeux soient plus rapides)

        Rect rectGauche(0, 0, (faces[indicePlusGrand].width/2)-1, faces[indicePlusGrand].height-1); // Rectangle pour l'oeil gauche (visage coupé en 2 dans la hauteur)
        Rect rectDroite((faces[indicePlusGrand].width)/2, 0 ,(faces[indicePlusGrand].width/2)-1,faces[indicePlusGrand].height-1); // Rectangle pour l'oeil droit (visage coupé en 2 dans la hauteur)

        Mat zoneGauche = zone(rectGauche); // On créé une image contenant la moité gauche du visage
        Mat zoneDroite = zone(rectDroite); // idem pour le coté droit.

        /*bool smile = detectSmile(zone); // détection d'un éventuel sourire.
        bool leftEye = detectLeftEye(zoneGauche); // détection oeil gauche.
        bool rightEye = detectRightEye(zoneDroite); // détection oeil droit.*/


        displaySmiley(detectSmile(zone)/* détection du sourire*/, detectLeftEye(zoneGauche)/* détection de l'oeil gauche*/, detectRightEye(zoneDroite)/* détection de l'oeil droit*/);


        rectangle(frame, faces[indicePlusGrand], CV_RGB(0, 0,0), 2); // Dessine un rectangle autour du visage détecté.
        faceCenterX = faces[indicePlusGrand].x +0.5*faces[indicePlusGrand].width; // calcule l'abscisse du centre du visage
        faceCenterY = faces[indicePlusGrand].y + 0.5*faces[indicePlusGrand].height; // calcule l'ordonnée du centre du visage
        Point centreVisage(faceCenterX, faceCenterY); // définit un point.
        circle(frame, centreVisage, 2, CV_RGB(0, 0,0), 2, 8,0 ); // trace un cercle autour de ce point.

        // idée pour plus tard : on lisse la valeur sur 5-10 images pour éviter les changements brusques à cause d'un faux positif

        handleServo(faceCenterX, faceCenterY);

    }else { // Si on a pas réussi à identifier un visage, on affiche une croix sur le panneau led.
        // pas de visage
        uint8_t noFace[8][8][3] = {
            {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
            {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
            {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
            {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {255, 0,   0}, {255, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
            {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {255, 0,   0}, {255, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
            {{0,   0,   0}, {0,   0,   0}, {255,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 255, 0,   0}, {  0, 0,  0}, {  0, 0, 0}},
            {{0,   0,   0}, {255,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  255, 0,  0}, {  0, 0, 0}},
            {{0,   0,   0}, {0,   0,   0}, {0,  0,   0}, {0, 0,   0}, {0, 0,   0}, { 0, 0,   0}, {  0, 0,  0}, {  0, 0, 0}}
        };

        for(int k=0; k<8; k++){
                for(int l=1; l<9; l++){
                    pixel = carte.ConvertirRGB565(noFace[k][l-1]);
                    carte.AllumerPixel(k,l,pixel);
                }
         }
    }
return frame;
}

/*
 *Fonction qui s'occupe de capturer une image et de l'afficher dans le label associé.
 */
void ProjetSY25main::capturePicture(){

    camera.grab(); // On capture une image
    camera.retrieve(image); // On stocke l'image dans une image (sous forme de Mat)
    flip(image,image,0); // On la retourne (à l'envers par défaut)
    detectFace(image); // On fait toutes les détections.
    QImage image2 = cvMatToQImage(image); // On convertit l'image en QImage
    photoLabel->setPixmap(QPixmap::fromImage(image2)); // Puis on l'affiche dans l'interface utilisateur.
}

/*
 * appelé lors de click sur le bouton takePic
 * Fonctionne uniquement quand on ne capture pas de vidéo (sinon le bouton est grisé/désactivé).
 */
void ProjetSY25main::on_takepicBtn_clicked(){

    if(camera.open()){ // si la caméra s'est bien ouverte.
        capturePicture();
        camera.release();
     }
}
/*
 * appelé lors de click sur le bouton Vidéo
 */
void ProjetSY25main::on_videoBtn_clicked(){

    takepicBtn->setEnabled(false); // On désactive le bouton pour prendre une photo.
    initPort(); // On initialise la liaison série
    if(camera.open()){ // si la caméra s'est bien ouverte.
           videoBtn->setText("Stop");
           QTimer *timer = new QTimer();
           connect(timer, SIGNAL(timeout()), this, SLOT(capturePicture())); // lorsqu'on arrive à la fin du timer on prend une photo
           timer->setInterval(120); // définition de l'intervalle du timer (on prendra une photo toutes les 120 ms).
           timer->start(); // on démarre le timer
    }else{
        videoBtn->setText("Video");
        takepicBtn->setEnabled(true); // On réactive le bouton pour prendre une photo
        camera.release(); // On libère la caméra.
    }
}

/*
 * appelé lors de click sur le bouton exit
 * Il vient quitter l'application.
 */
void ProjetSY25main::on_exitBtn_clicked(){

     close(); // On quitte le programme.
}


