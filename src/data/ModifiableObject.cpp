#include "GoProOverlay/data/ModifiableObject.h"

#include "GoProOverlay/utils/misc/MiscUtils.h"
#include <spdlog/spdlog.h>

namespace gpo
{

    // init static member
    bool ModifiableObject::globalShowModificationCallStack_ = false;

    ModifiableObject::ModifiableObject(
            const std::string_view &className,
            bool supportsApplyingModifications,
            bool supportsSavingModifications)
     : className_(className)
     , supportsApplyingModifications_(supportsApplyingModifications)
     , supportsSavingModifications_(supportsSavingModifications)
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
     , supportsApplyingModifications_(other.supportsApplyingModifications_)
     , supportsSavingModifications_(other.supportsSavingModifications_)
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

        if (supportsSavingModifications_ && hasSavableEdits_)
        {
            spdlog::warn(
                "{}<{}> was destructed before changes were saved",
                className_,
                (void*)this);
        }
        else if (supportsApplyingModifications_ && hasApplyableEdits_)
        {
            spdlog::warn(
                "{}<{}> was destructed before changes were applied",
                className_,
                (void*)this);
        }
    }

    const std::string_view &
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

    void
    ModifiableObject::markObjectModified(
        bool needsApply,
        bool needsSave)
    {
        if (showModificationCallStack_ || globalShowModificationCallStack_)
        {
            // print the call stack that caused this object to be modified
            spdlog::info("{}() - {}<{}>",__func__,className(),(void*)this);
            utils::misc::printCallStack(stdout,10);
        }

        if (needsApply)
        {
            if (supportsApplyingModifications_)
            {
                hasApplyableEdits_ = hasApplyableEdits_ || needsApply;
            }
            else
            {
                spdlog::error(
                    "{}<{}> marked as 'needsApply' but it doesn't support applying modifications",
                    className(),
                    (void*)this);
            }
        }
        if (needsSave)
        {
            if (supportsSavingModifications_)
            {
                hasSavableEdits_ = hasSavableEdits_ || needsSave;
            }
            else
            {
                spdlog::error(
                    "{}<{}> marked as 'needsSave' but it doesn't support saving modifications",
                    className(),
                    (void*)this);
            }
        }
        for (auto &observer : observers_)
        {
            observer->onModified(this);
        }
    }
    
    bool
    ModifiableObject::applyModifications(
        bool unnecessaryIsOkay)
    {
        if ( ! supportsApplyingModifications_)
        {
            spdlog::error(
                "applyModifications() called, but {}<{}> does not support applying modifications",
                className_,
                (void*)this);
            return false;
        }
        else if ( ! hasApplyableEdits_ && ! unnecessaryIsOkay)
        {
            spdlog::warn(
                "applyModifications() called, but {}<{}> doesn't have any applyable modifications",
                className_,
                (void*)this);
            return false;
        }

        bool applyOkay = subclassApplyModifications(unnecessaryIsOkay);
        if (applyOkay)
        {
            hasApplyableEdits_ = false;
            for (auto &observer : observers_)
            {
                observer->onModificationsApplied(this);
            }
        }
        return applyOkay;
    }

    bool
    ModifiableObject::saveModifications(
        bool unnecessaryIsOkay)
    {
        if (supportsApplyingModifications_ && hasApplyableEdits_)
        {
            applyModifications();
        }

        if ( ! supportsSavingModifications_)
        {
            spdlog::error(
                "saveModifications() called, but {}<{}> does not support saving modifications",
                className_,
                (void*)this);
            return false;
        }
        else if ( ! hasSavableEdits_ && ! unnecessaryIsOkay)
        {
            spdlog::warn(
                "applyModifications() called, but {}<{}> doesn't have any savable modifications",
                className_,
                (void*)this);
            return false;
        }

        bool saveOkay = subclassSaveModifications(unnecessaryIsOkay);
        if (saveOkay)
        {
            hasSavableEdits_ = false;
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

    bool
    ModifiableObject::subclassApplyModifications(
        bool unnecessaryIsOkay)
    {
        spdlog::warn(
            "ModifiableObject's default apply was called on {}. Subclass should"
            " override 'bool ModifiableObject::subclassApplyModifications()' if"
            " they want to support applied changes tracking.",
            className_);
        return true;
    }

    bool
    ModifiableObject::subclassSaveModifications(
        bool unnecessaryIsOkay)
    {
        spdlog::warn(
            "ModifiableObject's default save was called on {}. Subclass should"
            " override 'bool ModifiableObject::subclassSaveModifications()' if"
            " they want to support save changes tracking.",
            className_);
        return true;
    }

}