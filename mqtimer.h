#ifndef MQTIMER_H
#define MQTIMER_H
#include <QDebug>
#include <QtCore/QDebug>
#include <QTimer>

#define LOCK_ON_FOCUS_LOST_DELAY 5000

typedef struct {
   QTimer *tim;            ///< QTimer object
   int beatsCtr;           ///< Divider / counter
   static int autoReload;  ///< Value for autoreloading the divider/ctr

   /** Initialize or change the autoreload value */
   void reload(int reloadVal = autoReload) {
      autoReload = reloadVal;
      beatsCtr = reloadVal;
   }

   /** Zero-check this member for time division */
   int roundCheck() {
      if (! beatsCtr) {
         reload();
         return 0;
      }
      else
         return beatsCtr--;
   }

   /** Slotable ? No... */
   void setEnabled(bool onoff = true) {
      if (onoff)  tim->start();
      else        tim->stop();
   }

   /* ===================================== */

   //      void MQTimer () {
   //         tim  = new QTimer();
   //      }

   /** Especially implemented for Power:: autoLock countdown */
   void startCountdown(int delay_ms =
         LOCK_ON_FOCUS_LOST_DELAY) {
      tim->setSingleShot( true );
      tim->setInterval( delay_ms );
      tim->start();
   }

   void cancelCountdown() {
      tim->stop();
   }

   int getLockOnFocusLostDelay() {
      return (int) LOCK_ON_FOCUS_LOST_DELAY;
   }

} MQTimer_t;

#endif // MQTIMER_H
