#ifndef QMODIFIABLEOBJECTOBSERVER_H
#define QMODIFIABLEOBJECTOBSERVER_H

#include <QWidget>

#include "GoProOverlay/data/ModifiableObject.h"

/**
 * A class that manages a QWidget's title based on observing a ModifiableObject
 * 
 * The default title format will be the following.
 *   Format: "MyDocument.txt[*] - Document Editor"
 * Where the widget's QWidget::windowFilePath() returns "MyDocument.txt", and
 * the titleBasename is "Document Editor"
*/
class QModifiableObjectObserver : private gpo::ModifiableObjectObserver
{
public:
    QModifiableObjectObserver();

    ~QModifiableObjectObserver();

    void
    bindWidget(
        QWidget *widget,
        const QString &titleBaseName = "");

    void
    bindModifiable(
        gpo::ModifiableObject *modifiable);
    
    void
    releaseModifiable();

    const QString &
    titleBaseName() const;

    void
    setTitleBaseName(
        const QString &titleBaseName);

private:
    void
    updateWindowTitle();

    void
    onModified(
        gpo::ModifiableObject *modifiable) override;
    
    void
    onModificationsApplied(
        gpo::ModifiableObject *modifiable) override;
    
    void
    onModificationsSaved(
        gpo::ModifiableObject *modifiable) override;
    
    void
    onSavePathChanged(
        gpo::ModifiableObject *modifiable) override;

    void
    onBeforeDestroy(
        gpo::ModifiableObject *modifiable) override;

private:
    // normally something simple simple like "Document Editor"
    // will get displayed like "MyDocument.txt[*] - Document Editor"
    QString titleBaseName_;

    // the widget we will manage the title for
    QWidget *widget_;

    // the modifiable object we're observing
    gpo::ModifiableObject *modifiable_;

};

#endif
