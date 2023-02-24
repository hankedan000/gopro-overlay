#pragma once

#include <filesystem>
#include <unordered_set>

namespace gpo
{
    // forward declaration
    class ModifiableObject;

    class ModifiableObjectObserver
    {
    public:
        virtual
        void
        onModified(
            ModifiableObject *modifiable)
        {}
        
        virtual
        void
        onModificationsApplied(
            ModifiableObject *modifiable)
        {}
        
        virtual
        void
        onModificationsSaved(
            ModifiableObject *modifiable)
        {}
    };

    class ModifiableObject
    {
    public:
        ModifiableObject();

        void
        setSavePath(
            const std::filesystem::path &path);
        
        const std::filesystem::path &
        getSavePath() const;

        /**
         * @return
         * true if the object supports applying changes
         */
        virtual
        bool
        isApplyable() const;

        /**
         * @return
         * true if the object supports saving changes
         */
        virtual
        bool
        isSaveable() const;

        virtual
        void
        markObjectModified();
        
        /**
         * @return
         * true if the changes were applied, false otherwise
         */
        bool
        applyModifications();

        /**
         * @return
         * true if the changes were saved, false otherwise
         */
        bool
        saveModifications();

        /**
         * This is like calling setSavePath() and saveModifications() in one
         * 
         * @return
         * true if the changes were saved, false otherwise
         */
        bool
        saveModificationsAs(
            const std::filesystem::path &path);
        
        /**
         * @return
         * true if there are modifications waiting to be applied and the
         * object supports apply changes
         */
        bool
        hasApplyableModifications() const;

        /**
         * @return
         * true if there are modifications waiting to be saved and the
         * object supports saving changes
         */
        bool
        hasSavableModifications() const;

        void
        addObserver(
            ModifiableObjectObserver *observer);

        void
        removeObserver(
            ModifiableObjectObserver *observer);

    protected:
        /**
         * Subclasses can call this method to clear 'hasApplyableEdits_'
         */
        void
        clearNeedsApply();

        /**
         * Subclasses can call this method to clear 'hasSavableEdits_'
         */
        void
        clearNeedsSave();

        /**
         * Subclass should override this method to apply any modifications
         * 
         * @return
         * true if the apply was successful, false otherwise
         */
        virtual
        bool
        subclassApplyModifications() = 0;

        /**
         * Subclass should override this method to save any modifications
         * 
         * @return
         * true if the save was successful, false otherwise
         */
        virtual
        bool
        subclassSaveModifications() = 0;

    private:
        // true if the object has been modified since last "apply"
        bool hasApplyableEdits_;

        // true if the object has been modified since last "save"
        bool hasSavableEdits_;

        // location to save the object to
        std::filesystem::path savePath_;

        std::unordered_set<ModifiableObjectObserver *> observers_;

    };
}