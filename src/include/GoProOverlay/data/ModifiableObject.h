#pragma once

#include <filesystem>
#include <string>
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

        virtual
        void
        onSavePathChanged(
            ModifiableObject *modifiable)
        {}

        virtual
        void
        onBeforeDestroy(
            ModifiableObject *modifiable)
        {}
    };

    class ModifiableObject
    {
    public:
        /**
         * Constructor
         * 
         * @param[in] className
         * the object's class name. useful for debugging
         */
        ModifiableObject(
            const std::string &className);

        virtual
        ~ModifiableObject();

        const std::string &
        className() const;

        void
        setSavePath(
            const std::filesystem::path &path);
        
        const std::filesystem::path &
        getSavePath() const;

        /**
         * Default behavior just returns true.
         * Subclass can override this method to provide additional checks
         * or to prevent the object from apply changes all together.
         * 
         * @return
         * true if the object's applyModifications() method is callable.
         * false otherwise.
         */
        virtual
        bool
        isApplyable(
            bool noisy = true) const;

        /**
         * Default behavior just checks if the save path has been set.
         * Subclass can override this method to provide additional checks
         * or to prevent the object from being saved all together.
         * 
         * @return
         * true if the object's saveModifications() method is callable.
         * false otherwise.
         */
        virtual
        bool
        isSavable(
            bool noisy = true) const;

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
        const std::string className_;

        // true if the object has been modified since last "apply"
        bool hasApplyableEdits_;

        // true if the object has been modified since last "save"
        bool hasSavableEdits_;

        // location to save the object to
        std::filesystem::path savePath_;

        std::unordered_set<ModifiableObjectObserver *> observers_;

    };
}