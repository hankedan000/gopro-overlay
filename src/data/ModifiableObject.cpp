#include "GoProOverlay/data/ModifiableObject.h"

#include "GoProOverlay/utils/misc/MiscUtils.h"

#include <spdlog/spdlog.h>

namespace gpo
{

    // init static member
    bool ModifiableObject::globalShowModificationCallStack_ = false;

    ModifiableObject::ModifiableObject(
            const std::string &className)
     : className_(className)
     , hasApplyableEdits_(false)
     , hasSavableEdits_(false)
     , savePath_()
     , observers_()
     , showModificationCallStack_(false)
    {
    }

    ModifiableObject::ModifiableObject(
        const ModifiableObject &other)
     : className_(other.className_)
     , hasApplyableEdits_(other.hasApplyableEdits_)
     , hasSavableEdits_(other.hasApplyableEdits_)
     , savePath_(other.savePath_)
     , observers_()// don't copy observers. they should remain bound only to 'other'.
    {
    }

    ModifiableObject::~ModifiableObject()
    {
        // make a copy of the observers list in case the observer removes themself in
        // their onBeforeDestroy() callback. we don't want to be iterating the list
        // while it's being modified.
        auto observersCopy = observers_;
        for (auto &observer : observersCopy)
        {
            observer->onBeforeDestroy(this);
        }

        if (isSavable() && hasSavableModifications())
        {
            spdlog::warn(
                "{}<{}> was destructed before changes were saved",
                className_,
                (void*)this);
        }
        else if (isApplyable() && hasApplyableModifications())
        {
            spdlog::warn(
                "{}<{}> was destructed before changes were applied",
                className_,
                (void*)this);
        }
    }

    const std::string &
    ModifiableObject::className() const
    {
        return className_;
    }

    void
    ModifiableObject::setSavePath(
        const std::filesystem::path &path)
    {
        savePath_ = path;
        for (auto &observer : observers_)
        {
            observer->onSavePathChanged(this);
        }
    }
    
    const std::filesystem::path &
    ModifiableObject::getSavePath() const
    {
        return savePath_;
    }

    bool
    ModifiableObject::isApplyable(
            bool noisy) const
    {
        return true;
    }

    bool
    ModifiableObject::isSavable(
            bool noisy) const
    {
        if (savePath_.empty())
        {
            if (noisy)
            {
                spdlog::error(
                    "save path is not set. {}<{}> is not savable.",
                    className_,
                    (void*)this);
            }
            return false;
        }
        return true;
    }

    void
    ModifiableObject::markObjectModified()
    {
        if (showModificationCallStack_ || globalShowModificationCallStack_)
        {
            // print the call stack that caused this object to be modified
            spdlog::info("{}() - {}<{}>",__func__,className(),(void*)this);
            utils::misc::printCallStack(stdout,10);
        }

        hasApplyableEdits_ = true;
        hasSavableEdits_ = true;
        for (auto &observer : observers_)
        {
            observer->onModified(this);
        }
    }
    
    bool
    ModifiableObject::applyModifications()
    {
        if ( ! isApplyable())
        {
            spdlog::error(
                "{} does not support applying modifications",
                className_);
            return false;
        }
        else if ( ! hasApplyableModifications())
        {
            spdlog::warn(
                "applyModifications() called, but {}<{}> doesn't have any modifications",
                className_,
                (void*)this);
            return false;
        }

        bool applyOkay = subclassApplyModifications();
        if (applyOkay)
        {
            clearNeedsApply();
            for (auto &observer : observers_)
            {
                observer->onModificationsApplied(this);
            }
        }
        return applyOkay;
    }

    bool
    ModifiableObject::saveModifications()
    {
        if (hasApplyableModifications() && isApplyable())
        {
            applyModifications();
        }

        if ( ! isSavable())
        {
            spdlog::warn(
                "{} does not support saving modifications",
                className_);
            return false;
        }
        else if ( ! hasSavableModifications())
        {
            spdlog::warn(
                "saveModifications() called, but {}<{}> doesn't have any modifications",
                className_,
                (void*)this);
            return false;
        }

        bool saveOkay = subclassSaveModifications();
        if (saveOkay)
        {
            clearNeedsSave();
            for (auto &observer : observers_)
            {
                observer->onModificationsSaved(this);
            }
        }
        return saveOkay;
    }

    bool
    ModifiableObject::saveModificationsAs(
        const std::filesystem::path &path)
    {
        setSavePath(path);
        return saveModifications();
    }
    
    bool
    ModifiableObject::hasApplyableModifications() const
    {
        return hasApplyableEdits_;
    }

    bool
    ModifiableObject::hasSavableModifications() const
    {
        return hasSavableEdits_;
    }

    void
    ModifiableObject::addObserver(
        ModifiableObjectObserver *observer)
    {
        if (observer == nullptr)
        {
            spdlog::warn("can't add a null ModifiableObjectObserver. ignoring add.'");
            return;
        }
        observers_.insert(observer);
    }

    void
    ModifiableObject::removeObserver(
        ModifiableObjectObserver *observer)
    {
        observers_.erase(observer);
    }

    void
    ModifiableObject::setShowModificationCallStack(
        bool show)
    {
        showModificationCallStack_ = show;
    }

    void
    ModifiableObject::setGlobalShowModificationCallStack(
        bool show)
    {
        globalShowModificationCallStack_ = show;
    }

    //---------------------------------------------------------------
    // protected methods
    //---------------------------------------------------------------
    void
    ModifiableObject::clearNeedsApply()
    {
        hasApplyableEdits_ = false;
    }

    void
    ModifiableObject::clearNeedsSave()
    {
        hasSavableEdits_ = false;
    }

}