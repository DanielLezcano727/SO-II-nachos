
#include "channel.hh"
#include "lock.hh"
#include "system.hh"

Channel::Channel(const char* debugName) {
    name = debugName;
    s_busy = false;
    l_busy = false;

    lock_listener = new Lock("Listener lock");
    lock_sender = new Lock("Sender lock");

    cond_listener_lock = new Lock("cond_listener_lock");
    cond_sender_lock = new Lock("cond_sender_lock");
    sender = new Condition("sender_cond", cond_sender_lock);
    listener = new Condition("listener_cond", cond_listener_lock);
}

Channel::~Channel() {
    delete sender;
    delete listener;
    delete lock_sender;
    delete lock_listener;
    delete cond_sender_lock;
    delete cond_listener_lock;

}

void Channel::Send(int msg) {
    /* Adquirimos lock para evitar ser interrumpidos por otros senders */
    DEBUG('t', "%s hace send\n", currentThread->GetName());
    lock_sender->Acquire(); 
    DEBUG('t', "%s adquiere el lock\n", currentThread->GetName());
    /* Si no hay alguien escuchando, marcamos que estamos enviando y esperamos */
    if (!l_busy) {
        s_busy = true;
        DEBUG('t', "%s se va a dormir. Los listener estan vacios\n", currentThread->GetName());
        sender->Wait();
        DEBUG('t', "%s se despierta ya que hay un listener\n", currentThread->GetName());
    }

    /* Escribimos el mensaje y nos quitamos el estado de ocupado */
    buffer = msg;
    s_busy = false;

    /* Le avisamos al listener que puede leer el mensaje y esperamos a que lo lea */
    listener->Signal();
    DEBUG('t', "%s envia mensaje. Se duerme\n", currentThread->GetName());
    sender->Wait();
    DEBUG('t', "%s se despierta. El receptor leyo el mensaje\n", currentThread->GetName());

    /* Liberamos el lock para otro proceso pueda enviar mensajes */
    lock_sender->Release();
    DEBUG('t', "%s libera el lock\n", currentThread->GetName());
}

void Channel::Receive(int* msg) {
    DEBUG('t', "hace receive %s\n", currentThread->GetName());
    lock_listener->Acquire();
    DEBUG('t', "%s adquiere el lock\n", currentThread->GetName());
    l_busy = true;
    if (s_busy) {
        sender->Signal();
        DEBUG('t', "%s avisa que está escuchando. Sender esta busy\n", currentThread->GetName());
    }

    DEBUG('t', "%s espera mensaje\n", currentThread->GetName());
    listener->Wait();
    DEBUG('t', "%s recibió mensaje\n", currentThread->GetName());
    *msg = buffer;
    l_busy = false;
    
    sender->Signal();
    DEBUG('t', "%s envió signal de confirmación\n", currentThread->GetName());
    lock_listener->Release();
    DEBUG('t', "%s liberó lock\n", currentThread->GetName());
}