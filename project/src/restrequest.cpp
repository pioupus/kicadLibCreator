#include "restrequest.h"
#include <QNetworkRequest>
#include <QDebug>
#include <QMessageBox>
#include <QAuthenticator>
#include <QSslError>
#include <QApplication>
#include <assert.h>

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif
static void mySleep(int ms)
{
    assert(ms > 0);

#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QUrlQuery>
#endif

RESTRequest::RESTRequest(QObject *parent): QObject(parent),qnam(parent)
{


}

RESTRequest::~RESTRequest()
{

}

void RESTRequest::startRequest(QString url,QMultiMap<QString, QString>params, QBuffer *result){
    return startRequest_raw(url,params, result,-1);
}

void RESTRequest::startRequest_raw(QUrl url,QMultiMap<QString, QString>params, QBuffer *result, int sessionIndex)
{
    bool recursionCall = sessionIndex > -1;
#if 1

    qDebug() << "startRequest_raw";
    QUrl query(url);

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)

    while(i.hasNext()) {
        i.next();
        query.addEncodedQueryItem(QUrl::toPercentEncoding(i.key()), QUrl::toPercentEncoding(i.value()));
    }

#else
    QUrlQuery q;
    QList<QString> keys = params.keys();
    for (int k=0;k<keys.count();k++){
        QList<QString> values = params.values(keys[k]);
        for (int v=0;v<values.count();v++){
            q.addQueryItem(QUrl::toPercentEncoding(keys[k]), QUrl::toPercentEncoding(values[v]));
        }
    }

    query.setQuery(q);

#endif


    QNetworkRequest request(query);
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));

    RequestSession* sessionItem;
    if (!recursionCall){
        sessionItem = new RequestSession();
    }else{
        sessionItem = sessions[sessionIndex];
    }
    sessionItem->reply = qnam.get(request);
    sessionItem->outFile = result;
    sessionItem->map = &params;
    result->open(QIODevice::WriteOnly);

    connect(sessionItem->reply, SIGNAL(finished()),
            this, SLOT(httpFinished()));
    connect(sessionItem->reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));
    connect(sessionItem->reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDataReadProgress(qint64,qint64)));

    sessions.append(sessionItem);
    sessionIndex = sessions.length();

    while(sessionItem->mutex){
        QApplication::processEvents(QEventLoop::AllEvents);
        mySleep(10);
    }
    if(!recursionCall){
        sessionItem->outFile->close();
        sessions.removeAt(sessionIndex);
        delete sessionItem;
    }
    qDebug() << "after wait 2";
#else
    (void)url;
    (void)params;
#endif
}

int RESTRequest::findSession(QObject *networkReply)
{

    for (int i=0;i<sessions.length();i++){
        if (sessions[i]->reply == networkReply){
            return i;
        }
    }
    return -1;
}

void RESTRequest::httpFinished()
{
    qDebug() << "httpFinished";
#if 1

     int sessionIndex = findSession(QObject::sender());
     if (sessionIndex == -1){
         return;
     }
    RequestSession* session = sessions[sessionIndex];

    QNetworkReply* reply = session->reply;

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
    } else if (!redirectionTarget.isNull()) {
        QUrl newUrl = reply->url().resolved(redirectionTarget.toUrl());

        reply->deleteLater();
        startRequest_raw(newUrl,*session->map,session->outFile, sessionIndex);
        session->mutex = false;
        return;

    } else {

        emit http_request_finished();
        //ready
    }


    reply->deleteLater();
    session->mutex = false;

#endif
}


void RESTRequest::httpReadyRead()
{
    qDebug() << "httpReadyRead";

#if 1
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    int sessionIndex = findSession(QObject::sender());
    if (sessionIndex == -1){
        return;
    }
    RequestSession* session = sessions[sessionIndex];

    session->outFile->write(session->reply->readAll());

#endif
}

void RESTRequest::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    qDebug() << "updateDataReadProgress";
 #if 1

    qDebug() << "progress total" << totalBytes;
    qDebug() << "progess read" << bytesRead;
#else
    (void)bytesRead;
    (void)totalBytes;
#endif

}

void RESTRequest::slotAuthenticationRequired(QNetworkReply*,QAuthenticator *authenticator)
{
    qDebug() << "slotAuthenticationRequired";

    authenticator->setUser("username");
    authenticator->setPassword("password");
}

#ifndef QT_NO_OPENSSL
void RESTRequest::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
{
    QString errorString;

    foreach (const QSslError &error, errors) {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }
    qDebug() << QString("One or more SSL errors has occurred: %1").arg(errorString);
    int sessionIndex = findSession(QObject::sender());
    if (sessionIndex == -1){
        return;
    }
   RequestSession* session = sessions[sessionIndex];


    session->mutex = false;
}
#endif

RequestSession::RequestSession()
{
    reply = NULL;
    mutex = true;
    outFile = NULL;
}
