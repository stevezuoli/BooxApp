
#include <QSharedMemory>
#include <QTime>

int main(int argc, char *argv[])
{
    QSharedMemory frame_buffer("onyx-international-e-ink-screen-memory");

    if (!frame_buffer.attach())
    {
        qWarning("Could not attach.");
        return -1;
    }

    if (!frame_buffer.lock())
    {
        qWarning("Could not acquire the lock");
        return -1;
    }

    // Write data to controller.
    QTime t;
    t.start();
    unsigned char *p  = reinterpret_cast<unsigned char *>(frame_buffer.data());

    t.elapsed();
    frame_buffer.unlock();
    return 0;
}
