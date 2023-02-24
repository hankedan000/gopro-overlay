#include "GoProOverlay/data/ModifiableObject.h"

#include <spdlog/spdlog.h>

namespace gpo
{
    ModifiableObject::ModifiableObject()
     : hasApplyableEdits_(false)
     , hasSavableEdits_(false)
     , savePath_()
     , observers_()
    {
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
    ModifiableObject::isApplyable() const
    {
        return true;
    }

    bool
    ModifiableObject::isSaveable() const
    {
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
            spdlog::error("object doesn't support applying modifications");
            return false;
        }
        else if ( ! hasApplyableModifications())
        {
            spdlog::warn("applyModifications() called, but object doesn't have any modifications");
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
        if ( ! isSaveable())
        {
            spdlog::error("object doesn't support saving modifications");
            return false;
        }
        else if ( ! hasSavableModifications())
        {
            spdlog::warn("saveModifications() called, but object doesn't have any modifications");
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