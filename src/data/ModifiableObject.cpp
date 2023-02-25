#include "GoProOverlay/data/ModifiableObject.h"

#include <spdlog/spdlog.h>

namespace gpo
{
    ModifiableObject::ModifiableObject(
            const std::string &className)
     : className_(className)
     , hasApplyableEdits_(false)
     , hasSavableEdits_(false)
     , savePath_()
     , observers_()
    {
    }

    ModifiableObject::~ModifiableObject()
    {
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
        if (isApplyable())
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
        observers_.insert(observer);
    }

    void
    ModifiableObject::removeObserver(
        ModifiableObjectObserver *observer)
    {
        observers_.erase(observer);
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