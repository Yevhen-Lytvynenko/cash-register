#include "MacroManager.h"
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <QDir>
#include <vector>

#ifdef Q_OS_LINUX
#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#endif

class MacroManager::RecorderThread : public QThread {
public:
    QString filePath;
    std::atomic<bool> running{false};

    void run() override {
#ifdef Q_OS_LINUX
        std::vector<int> fds;
        QDir dir("/dev/input");
        QStringList entries = dir.entryList(QStringList() << "event*", QDir::System);

        for (const QString& entry : entries) {
            QString path = "/dev/input/" + entry;
            int fd = open(path.toStdString().c_str(), O_RDONLY | O_NONBLOCK);
            if (fd >= 0) fds.push_back(fd);
        }

        if (fds.empty()) return;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            for (int fd : fds) close(fd);
            return;
        }
        QTextStream out(&file);

        QElapsedTimer timer;
        timer.start();
        qint64 lastTime = 0;

        running = true;
        while (running) {
            fd_set readSet;
            FD_ZERO(&readSet);
            int maxFd = -1;

            for (int fd : fds) {
                FD_SET(fd, &readSet);
                if (fd > maxFd) maxFd = fd;
            }

            struct timeval tv{0, 50000};
            int ret = select(maxFd + 1, &readSet, nullptr, nullptr, &tv);

            if (ret > 0) {
                for (int fd : fds) {
                    if (FD_ISSET(fd, &readSet)) {
                        struct input_event ev;
                        while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
                            qint64 currentTime = timer.elapsed();
                            qint64 delay = (lastTime == 0) ? 0 : (currentTime - lastTime);
                            lastTime = currentTime;

                            out << delay << " " << ev.type << " " << ev.code << " " << ev.value << "\n";
                            out.flush();
                        }
                    }
                }
            }
        }

        for (int fd : fds) close(fd);
        file.close();
#endif
    }
};

class MacroManager::PlayerThread : public QThread {
public:
    QString filePath;
    bool loop{false};
    std::atomic<bool> running{false};

    void run() override {
#ifdef Q_OS_LINUX
        int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (fd < 0) return;

        ioctl(fd, UI_SET_EVBIT, EV_KEY);
        for (int i = 0; i < 256; i++) ioctl(fd, UI_SET_KEYBIT, i);

        ioctl(fd, UI_SET_EVBIT, EV_REL);
        ioctl(fd, UI_SET_RELBIT, REL_X);
        ioctl(fd, UI_SET_RELBIT, REL_Y);
        ioctl(fd, UI_SET_RELBIT, REL_WHEEL);

        ioctl(fd, UI_SET_EVBIT, EV_ABS);
        for (int i = 0; i < ABS_MAX; i++) ioctl(fd, UI_SET_ABSBIT, i);

        ioctl(fd, UI_SET_EVBIT, EV_SYN);

        struct uinput_user_dev uidev = {};
        snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Virtual POS Device");
        uidev.id.bustype = BUS_USB;
        uidev.id.vendor  = 0x1234;
        uidev.id.product = 0x5678;
        uidev.id.version = 1;

        write(fd, &uidev, sizeof(uidev));
        ioctl(fd, UI_DEV_CREATE);

        usleep(100000);

        running = true;
        do {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) break;
            QTextStream in(&file);

            while (!in.atEnd() && running) {
                QString line = in.readLine();
                QStringList parts = line.split(" ", Qt::SkipEmptyParts);

                if (parts.size() == 4) {
                    qint64 delay = parts[0].toLongLong();
                    uint16_t type = parts[1].toUShort();
                    uint16_t code = parts[2].toUShort();
                    int32_t value = parts[3].toInt();

                    if (delay > 0) {
                        QThread::msleep(delay);
                    }

                    struct input_event ev = {};
                    ev.type = type;
                    ev.code = code;
                    ev.value = value;
                    gettimeofday(&ev.time, nullptr);

                    write(fd, &ev, sizeof(ev));
                }
            }
            file.close();

        } while (loop && running);

        ioctl(fd, UI_DEV_DESTROY);
        close(fd);
#endif
    }
};

MacroManager::MacroManager(QObject* parent)
    : QObject(parent),
    m_recorderThread(new RecorderThread()),
    m_playerThread(new PlayerThread()) {}

MacroManager::~MacroManager() {
    stopRecording();
    stopPlaying();
    delete m_recorderThread;
    delete m_playerThread;
}

void MacroManager::startRecording(const QString& filePath) {
    if (m_recorderThread->isRunning()) return;
    m_recorderThread->filePath = filePath;
    m_recorderThread->start();
}

void MacroManager::stopRecording() {
    if (m_recorderThread->isRunning()) {
        m_recorderThread->running = false;
        m_recorderThread->wait();
    }
}

void MacroManager::startPlaying(const QString& filePath, bool loop) {
    if (m_playerThread->isRunning()) return;
    m_playerThread->filePath = filePath;
    m_playerThread->loop = loop;
    m_playerThread->start();
}

void MacroManager::stopPlaying() {
    if (m_playerThread->isRunning()) {
        m_playerThread->running = false;
        m_playerThread->wait();
    }
}
