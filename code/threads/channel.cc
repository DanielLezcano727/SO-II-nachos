
#include "channel.hh"
#include "lock.hh"
#include "system.hh"

Channel::Channel(const char* debugName) {
    name = debugName;

    lock_listener = new Lock("Listener lock");
    lock_sender = new Lock("Sender lock");

    sem1 = new Semaphore("sem1", 0);
    sem2 = new Semaphore("sem2", 0);
}

Channel::~Channel() {
    delete sender;
    delete listener;

    delete sem1;
    delete sem2;
}

void Channel::Send(int msg) {
    // Adquiero lock para evitar la entrada de otros senders
    lock_sender->Acquire();

    // Intento adquirir el primer semáforo, si llego primero me duermo
    // Si llego segundo paso directo, el listener está esperando
    sem1->P();
    buffer = msg;
    // Le cedo el segundo semáforo al listener, ya escribí mi mensaje
    sem2->V();

    // Libero el espacio de sender
    lock_sender->Release();
}

void Channel::Receive(int* msg) {
    // Adquiero lock para evitar la entrada de otros listeners
    lock_listener->Acquire();
    
    // Aviso de mi llegada, si llegué primero el aviso queda listo
    // si llegué segundo despierto al sender
    sem1->V();
    // Espero al mensaje del sender
    sem2->P();
    *msg = buffer;
    
    // Libero el espacio de listener
    lock_listener->Release();
}