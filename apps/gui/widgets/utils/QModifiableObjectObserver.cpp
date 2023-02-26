#include "utils/QModifiableObjectObserver.h"

#include <spdlog/spdlog.h>

//-----------------------------------------------------------------------
// public methods
//-----------------------------------------------------------------------
QModifiableObjectObserver::QModifiableObjectObserver()
    : titleBaseName_("")
    , widget_(nullptr)
    , modifiable_(nullptr)
{}

QModifiableObjectObserver::~QModifiableObjectObserver()
{
    releaseModifiable();
}

void
QModifiableObjectObserver::bindWidget(
    QWidget *widget,
    const QString &titleBaseName)
{
    spdlog::trace("{}({},\"{}\")",
        __func__,
        (void*)widget,
        titleBaseName.toStdString());
    widget_ = widget;
    setTitleBaseName(titleBaseName);
}

void
QModifiableObjectObserver::bindModifiable(
    gpo::ModifiableObject *modifiable)
{
    spdlog::trace("{}({})",
        __func__,
        (void*)modifiable);
    releaseModifiable();
    modifiable_ = modifiable;
    if (modifiable_)
    {
        modifiable_->addObserver(this);
    }
    updateWindowTitle();
}

void
QModifiableObjectObserver::releaseModifiable()
{
    if (modifiable_)
    {
        modifiable_->removeObserver(this);
    }
    modifiable_ = nullptr;
    updateWindowTitle();
}

const QString &
QModifiableObjectObserver::titleBaseName() const
{
    return titleBaseName_;
}

void
QModifiableObjectObserver::setTitleBaseName(
    const QString &titleBaseName)
{
    spdlog::trace("{}(\"{}\")",
        __func__,
        titleBaseName.toStdString());
    titleBaseName_ = titleBaseName;
    updateWindowTitle();
}

//-----------------------------------------------------------------------
// private methods
//-----------------------------------------------------------------------
void
QModifiableObjectObserver::updateWindowTitle()
{
    spdlog::trace("{}()",__func__);
    if (widget_)
    {
        auto filePath = widget_->windowFilePath();
        if (filePath.size() == 0)
        {
            widget_->setWindowTitle(QStringLiteral("[*]%1")
                .arg(titleBaseName_));
        }
        else
        {
            widget_->setWindowTitle(QStringLiteral("%1[*] - %2")
                .arg(filePath)
                .arg(titleBaseName_));
        }

        if (modifiable_)
        {
            widget_->setWindowModified(modifiable_->hasSavableModifications());
        }
    }
}

//-----------------------------------------------------------------------
// ModifiableObjectObserver callbacks
//-----------------------------------------------------------------------
void
QModifiableObjectObserver::onModified(
    gpo::ModifiableObject *modifiable)
{
    spdlog::trace("{}({})",
        __func__,
        (void*)modifiable);
    if (widget_)
    {
        widget_->setWindowModified(true);
    }
}

void
QModifiableObjectObserver::onModificationsApplied(
    gpo::ModifiableObject *modifiable)
{}

void
QModifiableObjectObserver::onModificationsSaved(
    gpo::ModifiableObject *modifiable)
{
    spdlog::trace("{}({})",
        __func__,
        (void*)modifiable);
    if (widget_)
    {
        widget_->setWindowModified(false);
    }
}

void
QModifiableObjectObserver::onSavePathChanged(
    gpo::ModifiableObject *modifiable)
{
    spdlog::trace("{}({})",
        __func__,
        (void*)modifiable);
    if (widget_)
    {
        widget_->setWindowFilePath(
            modifiable->getSavePath().c_str());
        updateWindowTitle();
    }
}

void
QModifiableObjectObserver::onBeforeDestroy(
    gpo::ModifiableObject *modifiable)
{
    spdlog::trace("{}({})",
        __func__,
        (void*)modifiable);
    releaseModifiable();
}
