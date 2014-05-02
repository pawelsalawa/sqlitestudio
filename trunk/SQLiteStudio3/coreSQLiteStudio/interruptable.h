#ifndef INTERRUPTABLE_H
#define INTERRUPTABLE_H

/**
 * @brief The interruptable interface
 *
 * Represents anything that does some work, that can be interrupted.
 */
class Interruptable
{
    public:

        /**
         * @brief Interrupts current execution.
         *
         * This method makes sense only when execution takes place in thread other, than the one calling this method.
         * It interrupts execution - in most cases instantly.
         * This method doesn't return until the interrupting is done.
         */
        virtual void interrupt() = 0;
};

#endif // INTERRUPTABLE_H
