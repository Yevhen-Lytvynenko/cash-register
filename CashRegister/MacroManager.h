#pragma once

#include <QObject>
#include <QThread>
#include <QString>
#include <atomic>

class MacroManager : public QObject {
    Q_OBJECT

public:
    explicit MacroManager(QObject* parent = nullptr);
    ~MacroManager() override;

    void startRecording(const QString& filePath);
    void stopRecording();
    void startPlaying(const QString& filePath, bool loop);
    void stopPlaying();

signals:
    void errorOccurred(const QString& message);

private:
    class RecorderThread;
    class PlayerThread;

    RecorderThread* m_recorderThread;
    PlayerThread* m_playerThread;
};
