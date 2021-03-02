
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QMessageBox>
int SET_POINT_RIEGO = 3600;

int i = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    enumerarPuertos();
    initwid();


}

void MainWindow::initwid()
{
    ui->spinBox_temp->setRange(20,90);
    ui->spinBox_hum->setRange(10,90);
    ui->spinBox_riego->setRange(1,49);
   // ui->progressBar->setRange(0, SET_POINT_RIEGO);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::enumerarPuertos()
{
    ui->comboBoxPuertos->clear();

    QList<QSerialPortInfo> puertos= QSerialPortInfo::availablePorts(); //defino puertos
    for (int i=0;i<puertos.size();i++) //.count es metodo de Qlist
    {
        ui->comboBoxPuertos->addItem(puertos.at(i).portName());
    }
}



void MainWindow::on_pushButton_7_clicked()
{
    if (!port.isOpen())
    {
        QString portName = ui->comboBoxPuertos->currentText(); //texto seleccionado
        if (portName.isEmpty()) {
            QMessageBox::critical(this, QString::fromLatin1("Error de conexión"), QString::fromLatin1("Seleccione un puerto válido"));
            return;
        }
        port.setPortName(portName);
        port.setBaudRate(QSerialPort::Baud9600);
        port.setDataBits(QSerialPort::Data8);
        port.setParity(QSerialPort::NoParity);
        port.setStopBits(QSerialPort::OneStop);
        port.setFlowControl(QSerialPort::NoFlowControl);
        if(port.open(QIODevice::ReadWrite) == true)
        {
            ui->pushButton_7->setText("Desconectar");
            ui->statusbar->setStyleSheet("font-weight: bold; color: black; background-color: lightgreen;");
            ui->statusbar->setText("CONECTADO");

           connect(&port, SIGNAL(readyRead()),this,SLOT(on_datosRecibidos()));
        }
        else
        {
            QMessageBox::critical(this,"Error","No se pudo abrir el puerto "+portName);
        }
    }
    else
    {
        port.close();
        ui->statusbar->setStyleSheet("font-weight: normal; color: white; background-color: red;");
        ui->statusbar->setText("Desconectado");
        ui->pushButton_7->setText("Conectar");

    }
}


void MainWindow::on_ButtonEnviar_clicked()
{
    static QString temp  ;
    static QString hum  ;
    static QString riego ;
    QByteArray datos;
    datos.append('$');
    datos.append('t');
    temp  = ui->spinBox_temp->cleanText();
    hum = ui->spinBox_hum->cleanText();
    riego = ui->spinBox_riego->cleanText();
    datos.append(temp);
    datos.append('h');
    datos.append(hum);
    datos.append('r');
    datos.append(riego);
    datos.append("#");
    port.write(datos.data(), datos.size());
   // ui->progressBar->setRange(0, (riego.toInt())*3600);
    SET_POINT_RIEGO = (riego.toInt())*3600;
}



void MainWindow::on_RefreshButton_clicked()
{
    enumerarPuertos();
}

void MainWindow::on_IluminacionButton_clicked()
{
    QString comando;

    if (port.isOpen()) {
        if (ilumON == false) {
            comando = "$I1#";
            port.putChar('$');
            port.putChar('I');
            port.putChar('1');
            port.putChar('#');
            ilumON = true;
            ui->IluminacionButton->setText("APAGAR ILUMINACION");
        } else {
            comando = "$I0#";
            port.putChar('$');
            port.putChar('I');
            port.putChar('0');
            port.putChar('#');
            ilumON = false;
            ui->IluminacionButton->setText("ENCENDER ILUMINACION");
        }
    }
}

void MainWindow::on_RiegoButton_clicked()
{
    QString comando;

    if (port.isOpen()) {
        if (riegoON == false) {
            comando = "$R1#";
            port.putChar('$');
            port.putChar('R');
            port.putChar('1');
            port.putChar('#');
            riegoON = true;
            ui->RiegoButton->setText("APAGAR RIEGO");
        } else {
            comando = "$R0#";
            port.putChar('$');
            port.putChar('R');
            port.putChar('0');
            port.putChar('#');
            riegoON = false;
            ui->RiegoButton->setText("ENCENDER RIEGO");
        }
    }
}

void MainWindow::on_VentButton_clicked()
{
    QString comando;

    if (port.isOpen()) {
        if (ventON == false) {
            comando = "$V1#";
            port.putChar('$');
            port.putChar('V');
            port.putChar('1');
            port.putChar('#');
            ventON = true;
            ui->VentButton->setText("APAGAR VENTILACION");
        } else {
            comando = "$V0#";
            port.putChar('$');
            port.putChar('V');
            port.putChar('0');
            port.putChar('#');
            ventON = false;
            ui->VentButton->setText("ENCENDER VENTILACION");
        }
    }
}


void MainWindow::on_datosRecibidos()
{
    QByteArray bytesRx;
    int cant = port.bytesAvailable(); // devuelve la cantidad de bytes disponibles
    bytesRx.resize(cant);

    port.read(bytesRx.data(), bytesRx.size()); // guarda en bytesRx los datos recibidos

    datosRecibidos.append(bytesRx);

    procesarDatosRecibidos();
}


void MainWindow::procesarDatosRecibidos()
{
    static unsigned int estadoRx = ESPERO_MENSAJE;
    static int flag = 2, indexriego;
    int RiegoTime = 0;
    static char valort[] = {'0', '0'};
    QString HTime, STime, MTime;

    for (int i = 0; i < datosRecibidos.count(); i++) {
        unsigned char dato = datosRecibidos.at(i);
        switch(estadoRx) {
        case ESPERO_MENSAJE:
            if(dato == '#') {

                estadoRx = RECIBO_PARAMETRO;
            }
            break;
        case RECIBO_PARAMETRO:
            if ( dato == 'T')
            {
                estadoRx = RECIBO_DATOS_MSB;
                flag = 0;
            }
            else if ( dato == 'H' )
            {
                estadoRx = RECIBO_DATOS_MSBH;
                flag = 1;
            }
            else if ( dato == 'S' )
            {
                estadoRx = RECIBO_STATUS;
            }
            else if ( dato == 'r' )
            {
                estadoRx = RECIBO_TIEMPO_RIEGO;
            }
            else
               estadoRx = FIN_DE_TRAMA;
            break;
        case RECIBO_STATUS:
            if (dato == 'I')
                estadoRx = RECIBO_STATUS_ILUM;
            else if (dato == 'R')
                estadoRx = RECIBO_STATUS_RIEGO;
            else if (dato == 'V')
                estadoRx = RECIBO_STATUS_VENT;
            break;
        case RECIBO_TIEMPO_RIEGO:
        {
            if ( dato == 'h' )
                estadoRx = RECIBO_HORAS;
            else if ( dato == 'm' )
                estadoRx = RECIBO_MINUTOS;
            else if ( dato == 's' )
                estadoRx = RECIBO_SEGUNDOS;
            else
                estadoRx = ESPERO_MENSAJE;
            break;
        }
        case RECIBO_HORAS:
        {
            if (dato == 'm')
            estadoRx = RECIBO_MINUTOS;

            else
                HTime.append(dato);

            break;
        }

        case RECIBO_MINUTOS:
        {
            if (dato == 's')
            estadoRx = RECIBO_SEGUNDOS;
            else
            MTime.append(dato);
            break;
         }
        case RECIBO_SEGUNDOS:
        {
            STime.append(dato);
            indexriego++;
            if (indexriego == 2)
            estadoRx = FIN_DE_TRAMA;
            break;
        }
        case RECIBO_STATUS_ILUM:
            if ( dato == '1')
            {
                ui->IluminacionButton->setText("APAGAR ILUMINACION");
                ilumON = true;
            }
            else if (dato == '0')
            {
                ui->IluminacionButton->setText("ENCENDER ILUMINACION");
                ilumON = false;
            }
            estadoRx = FIN_DE_TRAMA;
            break;
        case RECIBO_STATUS_RIEGO:
            if ( dato == '1')
            {
                ui->RiegoButton->setText("APAGAR RIEGO");
                riegoON = true;
            }
            else if (dato == '0')
            {
                ui->RiegoButton->setText("ENCENDER RIEGO");
                riegoON = false;
            }
            estadoRx = FIN_DE_TRAMA;
            break;
        case RECIBO_STATUS_VENT:
            if ( dato == '1')
            {
                ui->VentButton->setText("APAGAR VENTILACION");
                ventON = true;
            }
            else if (dato == '0')
            {
                ui->VentButton->setText("ENCENDER VENTILACION");
                ventON =false;
            }
            estadoRx = FIN_DE_TRAMA;
            break;
        case RECIBO_DATOS_MSB:
            valort[0] = (char) dato;
            estadoRx = RECIBO_DATOS_LSB;
            break;
        case RECIBO_DATOS_LSB:
            valort[1] = (char)(dato);
            estadoRx = FIN_DE_TRAMA;
            break;
        case RECIBO_DATOS_MSBH:
            valort[0] =  (char) dato;
            estadoRx = RECIBO_DATOS_LSBH;
            break;
        case RECIBO_DATOS_LSBH:
            valort[1] = (char) dato;
            estadoRx = FIN_DE_TRAMA;
            break;
        case FIN_DE_TRAMA:
            if(dato == '$') {
                if (flag == 1)
                ui->lcdNumber_2->display(valort);
                else if (flag == 0)
                ui->lcdNumber->display(valort);
                indexriego = 0;
                RiegoTime+=HTime.toInt() * 3600;
                RiegoTime+=STime.toInt();
                RiegoTime+=MTime.toInt()*60;
                if (RiegoTime > 0)
                {
                    int value = SET_POINT_RIEGO - RiegoTime;
                    //ui->progressBar->setValue(value);
                }
                ui->lcdNumber_3->display(HTime.toInt());
                ui->lcdNumber_4->display(MTime.toInt());
                ui->lcdNumber_5->display(STime.toInt());
                HTime.clear();
                STime.clear();
                MTime.clear();
                RiegoTime = 0;

            }
            estadoRx = ESPERO_MENSAJE;
            break;
        }
    }
    datosRecibidos.clear();
}


