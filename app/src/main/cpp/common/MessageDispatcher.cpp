#include "MessageDispatcher.h"
#include "../game/BaseGameEntity.h"
#include "../common/FrameCounter.h"
#include "../game/EntityManager.h"


using std::set;


//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
MessageDispatcher* MessageDispatcher::Instance()
{
  static MessageDispatcher instance; 
  
  return &instance;
}

//----------------------------- Dispatch ---------------------------------
//  
//  see description in header
//------------------------------------------------------------------------
void MessageDispatcher::Discharge(BaseGameEntity* pReceiver, const Telegram& telegram)
{
  if (!pReceiver->HandleMessage(telegram))
  {
   ;
  }
}

//---------------------------- DispatchMsg ---------------------------
//
//  given a message, a receiver, a sender and any time delay, this function
//  routes the message to the correct agent (if no delay) or stores
//  in the message queue to be dispatched at the correct time
//------------------------------------------------------------------------
void MessageDispatcher::DispatchMsg(float delay, int  sender, int receiver, int  msg, Vector2D dummy,
                                    void* AdditionalInfo = NULL)
{

  //get a pointer to the receiver
  BaseGameEntity* pReceiver = EntityMgr->GetEntityFromID(receiver);

  //make sure the receiver is valid
  if (pReceiver == NULL)
  {

    return;
  }
  
  //create the telegram
  Telegram telegram(0, sender, receiver, msg, dummy, AdditionalInfo);
  
  //if there is no delay, route telegram immediately                       
  if (delay <= 0.0)                                                        
  {
    //send the telegram to the recipient
    Discharge(pReceiver, telegram);
  }

  //else calculate the time when the telegram should be dispatched
  else
  {
    float CurrentTime = TickCounter->GetCurrentFrame();

    telegram.DispatchTime = CurrentTime + delay;

    //and put it in the queue
    PriorityQ.insert(telegram);   

  }
}

//---------------------- DispatchDelayedMessages -------------------------
//
//  This function dispatches any telegrams with a timestamp that has
//  expired. Any dispatched telegrams are removed from the queue
//------------------------------------------------------------------------
void MessageDispatcher::DispatchDelayedMessages()
{ 
  //first get current time
  float CurrentTime = TickCounter->GetCurrentFrame();

  //now peek at the queue to see if any telegrams need dispatching.
  //remove all telegrams from the front of the queue that have gone
  //past their sell by date
  while( !PriorityQ.empty() &&
	     (PriorityQ.begin()->DispatchTime < CurrentTime) && 
         (PriorityQ.begin()->DispatchTime > 0) )
  {
    //read the telegram from the front of the queue
    const Telegram& telegram = *PriorityQ.begin();

    //find the recipient
    BaseGameEntity* pReceiver = EntityMgr->GetEntityFromID(telegram.Receiver);

    #ifdef SHOW_MESSAGING_INFO
    debug_con << "\nQueued telegram ready for dispatch: Sent to " 
         << pReceiver->ID() << ". Msg is "<< telegram.Msg << "";
    #endif

    //send the telegram to the recipient
    Discharge(pReceiver, telegram);

	//remove it from the queue
    PriorityQ.erase(PriorityQ.begin());
  }
}



