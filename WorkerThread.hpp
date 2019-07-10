bool WorkerThread::CreateThread()
{
    if (!m_thread)
        m_thread = new thread(&WorkerThread::Process, this);
    return true;
}

void WorkerThread::Process()
{
    m_timerExit = false;
    std::thread timerThread(&WorkerThread::TimerThread, this);

    while (1)
    {
        ThreadMsg* msg = 0;
        {
            // Wait for a message to be added to the queue
            std::unique_lock<std::mutex> lk(m_mutex);
            while (m_queue.empty())
                m_cv.wait(lk);

            if (m_queue.empty())
                continue;

            msg = m_queue.front();
            m_queue.pop();
        }

        switch (msg->id)
        {
            case MSG_POST_USER_DATA:
            {
                ASSERT_TRUE(msg->msg != NULL);

                // Convert the ThreadMsg void* data back to a UserData*
                const UserData* userData = static_cast<const UserData*>(msg->msg);

                cout << userData->msg.c_str() << " " << userData->year << " on " << THREAD_NAME << endl;

                // Delete dynamic data passed through message queue
                delete userData;
                delete msg;
                break;
            }

            case MSG_TIMER:
                cout << "Timer expired on " << THREAD_NAME << endl;
                delete msg;
                break;

            case MSG_EXIT_THREAD:
            {
                m_timerExit = true;
                timerThread.join();

                delete msg;
                std::unique_lock<std::mutex> lk(m_mutex);
                while (!m_queue.empty())
                {
                    msg = m_queue.front();
                    m_queue.pop();
                    delete msg;
                }

                cout << "Exit thread on " << THREAD_NAME << endl;
                return;
            }

            default:
                ASSERT();
        }
    }
}

void WorkerThread::PostMsg(const UserData* data)
{
    ASSERT_TRUE(m_thread);

    ThreadMsg* threadMsg = new ThreadMsg(MSG_POST_USER_DATA, data);

    // Add user data msg to queue and notify worker thread
    std::unique_lock<std::mutex> lk(m_mutex);
    m_queue.push(threadMsg);
    m_cv.notify_one();
}

void WorkerThread::ExitThread()
{
    if (!m_thread)
        return;

    // Create a new ThreadMsg
    ThreadMsg* threadMsg = new ThreadMsg(MSG_EXIT_THREAD, 0);

    // Put exit thread message into the queue
    {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push(threadMsg);
        m_cv.notify_one();
    }

    m_thread->join();
    delete m_thread;
    m_thread = 0;
}

unsigned long WorkerThread::Process(void* parameter)
{
    MSG msg;
    BOOL bRet;

    // Start periodic timer
    MMRESULT timerId = timeSetEvent(250, 10, &WorkerThread::TimerExpired, 
                       reinterpret_cast<DWORD>(this), TIME_PERIODIC);

    while ((bRet = GetMessage(&msg, NULL, WM_USER_BEGIN, WM_USER_END)) != 0)
    {
        switch (msg.message)
        {
            case WM_DISPATCH_DELEGATE:
            {
                ASSERT_TRUE(msg.wParam != NULL);

                // Convert the ThreadMsg void* data back to a UserData*
                const UserData* userData = static_cast<const UserData*>(msg.wParam);

                cout << userData->msg.c_str() << " " << userData->year << " on " << THREAD_NAME << endl;

                // Delete dynamic data passed through message queue
                delete userData;
                break;
            }

            case WM_USER_TIMER:
                cout << "Timer expired on " << THREAD_NAME << endl;
                break;

            case WM_EXIT_THREAD:
                timeKillEvent(timerId);
                return 0;

            default:
                ASSERT();
        }
    }
    return 0;
}