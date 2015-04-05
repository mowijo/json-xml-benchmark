#include <math.h>

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "qjson/src/parser.h"
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>
#include <QXmlStreamReader>
#include <QtXml/QDomDocument>

class EntryGottenFromServer
{
public:
    QString name;
    QString md5;
    unsigned long size;
    QDateTime mtime;
};


QString * get(const QString & url, QMap<QString, QVariant> &stats)
{
   // qDebug() << url;
    QTime timer;
    QNetworkAccessManager nam;
    QNetworkRequest request;
    request.setUrl(url);
    timer.start();
    QNetworkReply *reply = nam.get(request);
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    stats["transfertime"] = timer.elapsed() *1000.0; //Micro seconds
    stats["size"] = reply->rawHeader("X-encodedSize");
    stats["encodeDuration"] = QString(reply->rawHeader("X-encodingDuration")).toFloat();    //Micro Seconds
    stats["encoder"] = reply->rawHeader("X-encoder");
    return new QString(reply->readAll());
}



double qt5JsonDecode(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();
    QJsonDocument doc = QJsonDocument::fromJson(data->toUtf8());
    QJsonArray array = doc.array();
    QJsonArray::const_iterator i;
    for (i = array.constBegin(); i != array.constEnd(); ++i)
    {
        QVariantMap object = (*i).toObject().toVariantMap();;
        EntryGottenFromServer *entry = new EntryGottenFromServer();
        entry->name = object["name"].toString();
        entry->md5 = object["md5"].toString();
        entry->mtime = object["mtime"].toDateTime();
        entry->size = object["size"].toInt();
        parsed << entry;
    }
    return timer.elapsed()*1000.0; //Micro seconds

}


double flavioDecode(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();

    QJson::Parser parser;
    bool ok;
    QVariantList list = parser.parse (data->toUtf8(), &ok).toList();

    QList<QVariant>::const_iterator i;
    for (i = list.constBegin(); i != list.constEnd(); ++i)
    {
        QVariantMap object = (*i).toMap();
        EntryGottenFromServer *entry = new EntryGottenFromServer();
        entry->name = object["name"].toString();
        entry->md5 = object["md5"].toString();
        entry->mtime = object["mtime"].toDateTime();
        entry->size = object["size"].toInt();
        parsed << entry;

    }
    double time = timer.elapsed()*1000.0; //Micro seconds
    return time;

}
double qt5DomDecodeWithAttributes(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();


    QDomDocument doc("mydocument");
    doc.setContent(data->toUtf8());
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            EntryGottenFromServer *entry = new EntryGottenFromServer;
            entry->name = e.attribute("name");
            entry->md5 = e.attribute("md5");
            entry->size = e.attribute("size").toInt();
            entry->mtime = QVariant(e.attribute("mtime")).toDateTime();
            parsed << entry;
        }
        n = n.nextSibling();
    }


    double time = timer.elapsed() *1000.0; //Micro seconds
    return time;
}


double qt5DomDecodeWithChildNodes(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();


    QDomDocument doc("mydocument");
    doc.setContent(data->toUtf8());
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull() && e.tagName() == "file") {
            EntryGottenFromServer *entry = new EntryGottenFromServer;

            QDomNode child = e.firstChild();
            while(!child.isNull()) {
                QDomElement ce = child.toElement();
                if( ! ce.isNull())
                {
                    if(ce.tagName() == "name")
                    {
                        entry->name = ce.text();
                    }
                    if(ce.tagName() == "mtime")
                    {
                        entry->mtime = QVariant(ce.text()).toDateTime();
                    }
                    if(ce.tagName() == "size")
                    {
                        entry->size = ce.text().toInt();
                    }
                    if(ce.tagName() == "md5")
                    {
                        entry->md5 = ce.text();
                    }
                }
                child = child.nextSibling();
            }
            parsed << entry;
        }
        n = n.nextSibling();
    }


    double time = timer.elapsed() *1000.0; //Micro seconds
    return time;
}


double qt5SaxDecodeWithChildNodes(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();

    class Handler : public QXmlDefaultHandler
    {
    public:

        enum ChildType
        {
            NAME,
            MTIME,
            SIZE,
            MD5
        };

        ChildType currentChild;
        EntryGottenFromServer *currententry;

        QList<EntryGottenFromServer*> *m_parsed;
        bool startElement(const QString & , const QString & localName, const QString & , const QXmlAttributes & )
        {
            if(localName == "file")
            {
                currententry = new EntryGottenFromServer;
            }
            else if(localName == "name")
            {
                this->currentChild = NAME;
            }
            else if(localName == "mtime")
            {
                this->currentChild = MTIME;
            }
            else if(localName == "size")
            {
                this->currentChild = SIZE;
            }
            else if(localName == "md5")
            {
                this->currentChild = MD5;
            }
            return true;
        }

        bool endElement(const QString &, const QString &localName, const QString &)
        {
            if(localName == "file")
            {
                m_parsed->append(this->currententry);
            }
            return true;
        }

        bool characters(const QString &ch)
        {
            switch(this->currentChild)
            {
            case NAME: this->currententry->name = ch; break;
            case MTIME: this->currententry->mtime = QVariant(ch).toDateTime(); break;
            case MD5: this->currententry->md5 = ch; break;
            case SIZE: this->currententry->size= ch.toInt(); break;
            }
            return true;
        }
    };

    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource();
    source->setData(*data);
    Handler *handler = new Handler;
    handler->m_parsed = &parsed;
    xmlReader.setContentHandler(handler);
    xmlReader.parse(source);
    double time = timer.elapsed() *1000.0; //Micro seconds
    return time;
}




double qt5StreamDecodeWithChildNodes(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();


    QXmlStreamReader xmlReader;
    xmlReader.addData(data->toUtf8());
    xmlReader.readNext();
    //Reading from the file
    EntryGottenFromServer *current;
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();
            if(name == "file")
            {
                current = new EntryGottenFromServer;
            }
            else if(name == "size")
            {
                current->size = xmlReader.readElementText().toInt();
            }
            else if(name == "name")
            {
                current->name = xmlReader.readElementText();
            }
            else if(name == "mtime")
            {
                current->mtime  = QVariant(xmlReader.readElementText()).toDateTime();
            }
            else if(name == "md5")
            {
                current->md5 = xmlReader.readElementText();
            }
        }

        if(xmlReader.isEndElement())
        {
            QString name = xmlReader.name().toString();
            if(name == "file")
            {
                parsed << current;
            }
        }
        xmlReader.readNext();
    }



    double time = timer.elapsed() *1000.0; //Micro seconds
    return time;
}


double qt5StreamDecodeWithAttributes(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();


    QXmlStreamReader xmlReader;
    xmlReader.addData(data->toUtf8());
    xmlReader.readNext();
    //Reading from the file
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();
            if(name == "file")
            {
                EntryGottenFromServer *e = new EntryGottenFromServer;
                e->name =  xmlReader.attributes().value("name").toString();
                e->size = xmlReader.attributes().value("size").toInt();
                e->md5  = xmlReader.attributes().value("md5").toString();
                e->mtime =QVariant(xmlReader.attributes().value("mtime").toString()).toDateTime();
                parsed << e;
            }
        }
        xmlReader.readNext();
    }



    double time = timer.elapsed() *1000.0; //Micro seconds
    return time;
}

double qt5SaxDecodeWithAttributes(QString *data)
{
    QList<EntryGottenFromServer*> parsed;
    QTime timer;
    timer.start();

    class Handler : public QXmlDefaultHandler
    {
    public:
        QList<EntryGottenFromServer*> *m_parsed;
        bool startElement(const QString & , const QString & localName, const QString & , const QXmlAttributes & atts)
        {
            if(localName == "file")
            {
                EntryGottenFromServer* current = new EntryGottenFromServer;
                current->name = atts.value("name");
                current->size = atts.value("size").toInt();
                current->md5  = atts.value("md5");
                current->mtime = QVariant(atts.value("mtime")).toDateTime();
                m_parsed->append(current);
            }
            return true;
        }
    };

    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource();
    source->setData(*data);
    Handler *handler = new Handler;
    handler->m_parsed = &parsed;
    xmlReader.setContentHandler(handler);
    xmlReader.parse(source);
    double time = timer.elapsed() *1000.0; //Micro seconds
    return time;
}


void decode(QString *data, QMap<QString, QVariant> &stats)
{
    QString mapkey = "decodeDuration";

    if(stats["decoder"] == "qt5-json")
    {
        stats[mapkey] = qt5JsonDecode(data);
    }
    if(stats["decoder"] == "flavio")
    {
        stats[mapkey] = flavioDecode(data);
    }
    if(stats["decoder"] == "qt5-sax-attributes")
    {
        stats[mapkey] = qt5SaxDecodeWithAttributes(data);
    }
    if(stats["decoder"] == "qt5-dom-attributes")
    {
        stats[mapkey] = qt5DomDecodeWithAttributes(data);
    }
    if(stats["decoder"] == "qt5-dom-childnodes")
    {
        stats[mapkey] = qt5DomDecodeWithChildNodes(data);
    }
    if(stats["decoder"] == "qt5-sax-childnodes")
    {
        stats[mapkey] = qt5SaxDecodeWithChildNodes(data);
    }
    if(stats["decoder"] == "qt5-stream-attributes")
    {
        stats[mapkey] = qt5StreamDecodeWithAttributes(data);
    }
    if(stats["decoder"] == "qt5-stream-childnodes")
    {
        stats[mapkey] = qt5StreamDecodeWithChildNodes(data);
    }



    stats["encodeDuration"] = stats["encodeDuration"].toDouble();
    stats["transfertime"] = stats["transfertime"].toDouble();
    stats["decodeDuration"] = stats["decodeDuration"].toDouble();
}


QString testEncoderDecoderSet(QPair<QString, QString> set)
{

    int SAMPLE_SIZE = 20;
    int ENTRIES = 2100;

    QList<QMap<QString, QVariant> > results;
    double encodesum = 0;
    double transfersum = 0;
    double decodesum = 0;
    double size;

    for(int i = 0; i < SAMPLE_SIZE; i++)
    {
        QMap<QString, QVariant> stats;        QString url = "http://scratch.localdomain/data.php?entries=%1&encoder=%2";
        url = url.arg(ENTRIES).arg(set.first);
        QString *data = get(url, stats);
        encodesum += stats["encodeDuration"].toDouble();
        transfersum += stats["transfertime"].toDouble();
        stats["decoder"] = set.second;
        decode(data, stats);
        decodesum += stats["decodeDuration"].toDouble();
        size = stats["size"].toDouble();
        delete data;
        results << stats;
    }

    double averageEncodeDuration = encodesum / SAMPLE_SIZE;
    double averageTransferTime = transfersum / SAMPLE_SIZE;
    double averateDecodeDuration = decodesum / SAMPLE_SIZE;


    QMap<QString, QVariant> sample;
    double sdEncodeDuration = 0;
    double sdDecodeDuration = 0;
    foreach(sample, results)
    {
        sdEncodeDuration += pow(sample["encodeDuration"].toDouble()-averageEncodeDuration, 2);
        sdDecodeDuration += pow(sample["decodeDuration"].toDouble()-averateDecodeDuration, 2);
    }

    sdEncodeDuration = sqrt(sdEncodeDuration / SAMPLE_SIZE);
    sdDecodeDuration = sqrt(sdDecodeDuration / SAMPLE_SIZE);


    QString report = set.second +", ";
    report += QString::number(averageEncodeDuration) + ", "+QString::number(sdEncodeDuration)+", ";
    report += QString::number(averageTransferTime) + ", 0, ";
    report += QString::number(size) + ", 0, ";
    report += QString::number(averateDecodeDuration) + ", "+QString::number(sdDecodeDuration);


    return report;
}

typedef QPair<QString, QString> Set;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QFile report("report.dat");
    report.open(QFile::WriteOnly);

    QList<Set>  sets;
/*
    sets << Set("buildin-json", "qt5-json");
    sets << Set("buildin-json", "flavio"); //https://github.com/flavio/qjson

    sets.append( Set("simplexml-attibutes", "qt5-sax-attributes"));
    sets.append( Set("simplexml-attibutes", "qt5-dom-attributes"));
    sets.append( Set("simplexml-attibutes", "qt5-stream-attributes"));
*/
    sets.append( Set("simplexml-childnodes", "qt5-sax-childnodes"));
    sets.append( Set("simplexml-childnodes", "qt5-dom-childnodes"));
    sets.append( Set("simplexml-childnodes", "qt5-stream-childnodes"));

    foreach(Set set, sets)
    {
        qDebug() << set.first + " and " +set.second;
        report.write(testEncoderDecoderSet(set).toUtf8());
        report.write("\n");
    }
    report.close();
}
