#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimeLine>


#define ESPERO_MENSAJE      0
#define RECIBO_PARAMETRO    1
#define RECIBO_STATUS       2
#define RECIBO_STATUS_ILUM  3
#define RECIBO_STATUS_RIEGO 4
#define RECIBO_STATUS_VENT  5
#define RECIBO_DATOS_MSB    6
#define RECIBO_DATOS_LSB    7
#define RECIBO_DATOS_MSBH   8
#define RECIBO_DATOS_LSBH   9
#define FIN_DE_TRAMA        10



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void enumerarPuertos();
    void on_ButtonConectar_clicked();
    void procesarDatosRecibidos();
    void initwid ();

private slots:
    void on_pushButton_7_clicked();

    void on_datosRecibidos();

    void on_ButtonEnviar_clicked();

    void on_RefreshButton_clicked();

    void on_IluminacionButton_clicked();

    void on_RiegoButton_clicked();

    void on_VentButton_clicked();



private:
    Ui::MainWindow *ui;
    QSerialPort port;
    QByteArray datosRecibidos;
    bool ilumON = false;
    bool riegoON = false;
    bool ventON = false;
    QList<double> dataADC;
    QTimeLine *timeLine = new QTimeLine(1000, this);

};

#endif // MAINWINDOW_H
