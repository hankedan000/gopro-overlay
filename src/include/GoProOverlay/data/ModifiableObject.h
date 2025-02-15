#pragma once

#include <filesystem>
#include <string_view>
#include <unordered_set>

namespace gpo
{
    // forward declaration
    class ModifiableObject;

    class ModifiableObjectObserver
    {
    public:
        virtual
        ~ModifiableObjectObserver() = default;

        virtual
        void
        onModified(
            ModifiableObject *modifiable)
        {
            // base impl does nothing. child class can override if needed.
        }
        
        virtual
        void
        onModificationsApplied(
            ModifiableObject *modifiable)
        {
            // base impl does nothing. child class can override if needed.
        }
        
        virtual
        void
        onModificationsSaved(
            ModifiableObject *modifiable)
        {
            // base impl does nothing. child class can override if needed.
        }

        virtual
        void
        onSavePathChanged(
            ModifiableObject *modifiable)
        {
            // base impl does nothing. child class can override if needed.
        }

        virtual
        void
        onBeforeDestroy(
            ModifiableObject *modifiable)
        {
            // base impl does nothing. child class can override if needed.
        }
    };

    class ModifiableObject
    {
    public:
        /**
         * Constructor
         * 
         * @param[in] className
         * the object's class name. useful for debugging
         * 
         * @param[in] supportsApplyingModifications
         * true if the class supports applying changes
         * (ie. the applyModifications() method is allowed to be called)
         * 
         * @param[in] supportsSavingModifications
         * true if the class supports saving modifications itself
         * (ie. the saveModifications() method is allowed to be called)
         */
        ModifiableObject(
            const std::string_view &className,
            bool supportsApplyingModifications,
            bool supportsSavingModifications);
        
        /**
         * Copy constructor
         * 
         * @param[in] other
         * the object to copy
         */
        ModifiableObject(
            const ModifiableObject &other);

        virtual
        ~ModifiableObject();

        const std::string_view &
        className() const;

        void
        setSavePath(
            const std::filesystem::path &path);
        
        const std::filesystem::path &
        getSavePath() const;

        virtual
        void
        markObjectModified(
            bool needsApply = true,
            bool needsSave = true);
        
        /**
         * @param[in] unnecessaryIsOkay
         * true if calling this method while there are no modifications
         * needing applied is okay (apply operation will be performed regardless).
         * otherwise, a warning log is produced and the apply operation is not performed.
         * 
         * @return
         * true if the changes were applied, false otherwise
         */
        bool
        applyModifications(
            bool unnecessaryIsOkay = false);

        /**
         * @param[in] unnecessaryIsOkay
         * true if calling this method while there are no modifications
         * needing saved is okay (save operation will be performed regardless).
         * otherwise, a warning log is produced and the save operation is not performed.
         * 
         * @return
         * true if the changes were saved, false otherwise
         */
        bool
        saveModifications(
            bool unnecessaryIsOkay = false);

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

        /**
         * Enable/Disable printing of a call stack when this object is marked
         * as modified via the markObjectModified() method.
         * 
         * @param[in] show
         * true to enable call stack printing. false to disable.
         */
        void
        setShowModificationCallStack(
            bool show);

        /**
         * Enable/Disable printing of a call stack when any ModifiableObjec
         * is marked as modified via the markObjectModified() method.
         * 
         * @param[in] show
         * true to enable call stack printing. false to disable.
         */
        static
        void
        setGlobalShowModificationCallStack(
            bool show);

        ModifiableObject *
        toModifiableObject()
        {
            return this;
        }

        const ModifiableObject *
        toModifiableObject() const
        {
            return this;
        }

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
        subclassApplyModifications(
            bool unnecessaryIsOkay);

        /**
         * Subclass should override this method to save any modifications
         * 
         * @return
         * true if the save was successful, false otherwise
         */
        virtual
        bool
        subclassSaveModifications(
            bool unnecessaryIsOkay);

    private:
        const std::string_view className_;

        // true if the class supports applying changes
        // (ie. the applyModifications() method is allowed to be called)
        bool supportsApplyingModifications_;

        // true if the class supports saving modifications itself
        // (ie. the saveModifications() method is allowed to be called)
        bool supportsSavingModifications_;

        // true if the object has been modified since last "apply"
        bool hasApplyableEdits_;

        // true if the object has been modified since last "save"
        bool hasSavableEdits_;

        // location to save the object to
        std::filesystem::path savePath_;

        std::unordered_set<ModifiableObjectObserver *> observers_;

        // set true to printout a callstack when this object is marked as modified
        bool showModificationCallStack_;

        // same concept as 'showModificationCallStack_' but this applies to all instances
        static bool globalShowModificationCallStack_;

    };
}