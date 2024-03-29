

QWidget* createVideoWidget()
{
	auto w = new VideoWidget();
	w->setWindowFlags(Qt::FramelessWindowHint);
	w->setWindowTitle("Manuel");
	w->show();
	return w;
}

/*  int runGuiTest(QApplication &a)
    {
    // Create a bunch of initial widgets.
    QList<QWidget*> widgets;
    for (auto i = 0; i < 5; ++i) {
    auto w = createVideoWidget();
    widgets.append(w);
    }

    // Update the frame of the widgets every X ms.
    auto pixmapsIndex = 0;
    QList<QPixmap> pixmaps;
    pixmaps.append(QPixmap::fromImage(QImage(QString(":/frame.jpg"))));
    pixmaps.append(QPixmap::fromImage(QImage(QString(":/frame2.jpg"))));
    pixmaps.append(QPixmap::fromImage(QImage(QString(":/frame3.jpg"))));
    QTimer t0;
    QObject::connect(&t0, &QTimer::timeout, [&pixmapsIndex, &pixmaps, &widgets] () {
    ++pixmapsIndex;
    if (pixmapsIndex >= pixmaps.size()) {
      pixmapsIndex = 0;
    }
    foreach (auto w, widgets) {
      //((VideoWidget*)w)->setFrame(pixmaps.at(pixmapsIndex));
    }
    });
    t0.start(66); // 66 = ~15fps


    // Arrange widgets on desktop in a grid.
    GridViewWidgetArranger grid;
    grid.setWidgets(widgets);
    grid.setColumnCount(5);
    grid.setColumnSpacing(10);
    grid.arrange();

    // Resize the grid every X ms.
    //  QTimer t1;
    //  QObject::connect(&t1, &QTimer::timeout, [&grid] () {
    //    grid.setColumnCount(grid.columnCount() + 1);
    //    grid.arrange();
    //  });
    //  t1.start(2000);


    // Use a widget which groups all widgets into a single one.
    //  VideoCollectionWidget coll;
    //  coll.setWidgets(widgets);
    //  coll.show();


    // Create more widgets over time.
    //  QTimer tWidgetCreator;
    //  QObject::connect(&tWidgetCreator, &QTimer::timeout, [&widgets, &grid] () {
    //    auto w = createVideoWidget();
    //    widgets.append(w);
    //    grid.setWidgets(widgets);
    //    grid.arrange();
    //  });
    //  tWidgetCreator.start(2000);

    return a.exec();
    }*/

/*  int runVideoCollectionTest(QApplication &a)
    {
    a.setQuitOnLastWindowClosed(true);

    QList<QWidget *> widgets;
    for (auto i = 0; i < 3; ++i) {
    auto w = new VideoWidget();
    widgets.append(w);
    }
    VideoCollectionWidget coll;
    coll.setWidgets(widgets);
    coll.setVisible(true);
    return a.exec();
    }*/

/*  int runHangoutViewTest(QApplication &a)
    {
    a.setQuitOnLastWindowClosed(true);

    HangoutViewWidget hang(nullptr);
    hang.setCameraWidget(new VideoWidget());
    hang.resize(800, 600);
    hang.setVisible(true);

    int addClientId = 1;
    int removeClientId = 1;
    int clientsCount = 0;

    // Add widgets.
    QTimer t;
    t.setInterval(2000);
    t.start();
    QObject::connect(&t, &QTimer::timeout, [&hang, &addClientId, &clientsCount] () {
    if (clientsCount >= 5)
      return;
    ClientEntity client;
    client.id = ++addClientId;
    ChannelEntity channel;
    channel.id = 0;
    hang.addClient(client, channel);
    ++clientsCount;
    });

    // Remove widgets.
    QTimer t2;
    t2.setInterval(5000);
    t2.start();
    QObject::connect(&t2, &QTimer::timeout, [&hang, &removeClientId, &clientsCount] () {
    ClientEntity client;
    client.id = ++removeClientId;
    ChannelEntity channel;
    channel.id = 0;
    hang.removeClient(client, channel);
    --clientsCount;
    });

    return a.exec();
    }*/

/*int runTileViewTest(QApplication& a)
{
	a.setQuitOnLastWindowClosed(true);

	TileViewWidget view;
	//view.setCameraWidget(new VideoWidget());
	view.resize(800, 600);
	view.setVisible(true);

	int addClientId = 1;
	int removeClientId = 1;
	int clientsCount = 0;

	// Add widgets.
	QTimer t;
	t.setInterval(2000);
	t.start();
	QObject::connect(&t, &QTimer::timeout, [&view, &addClientId, &clientsCount]()
	{
		if (clientsCount >= 5)
			return;
		ClientEntity client;
		client.id = ++addClientId;
		client.name = QString("Clientname #%1").arg(client.id);
		client.videoEnabled = true;
		ChannelEntity channel;
		channel.id = 0;
		view.addClient(client, channel);
		++clientsCount;
	});

	return a.exec();
}*/

/*int runTestClient(QApplication& a)
{
	a.setQuitOnLastWindowClosed(false);

	auto timer = new QTimer(nullptr);
	timer->setInterval(2000);
	timer->start();

	QList<NetworkClient*> ts3vconns;
	auto maxConns = ELWS::getArgsValue("--max", 1).toInt();
	auto serverAddress = ELWS::getArgsValue("--server-address", IFVS_SERVER_ADDRESS).toString();
	auto serverPort = ELWS::getArgsValue("--server-port", IFVS_SERVER_CONNECTION_PORT).toUInt();
	auto sendVideo = ELWS::getArgsValue("--video", "false").toBool();

	QObject::connect(timer, &QTimer::timeout, [timer, &maxConns, &ts3vconns, serverAddress, serverPort, sendVideo]()
	{
		// Stop creating new connections.
		if (maxConns != -1 && ts3vconns.size() >= maxConns)
		{
			timer->stop();
			return;
		}

		// Create a new connecion to the TS3VideoServer.
		auto ts3vc = new NetworkClient(nullptr);
		ts3vc->setMediaEnabled(true);
		ts3vc->connectToHost(QHostAddress(serverAddress), serverPort);
		ts3vconns.append(ts3vc);

		// Connected.
		QObject::connect(ts3vc, &NetworkClient::connected, [ts3vc, &ts3vconns, sendVideo]()
		{
			// Auth.
			auto reply = ts3vc->auth(QString("Test Client #%1").arg(ts3vconns.size()), "");
			QObject::connect(reply, &QCorReply::finished, [ts3vc, reply, sendVideo]()
			{
				reply->deleteLater();
				HL_DEBUG(HL, QString(reply->frame()->data()).toStdString());
				// Join channel.
				auto reply2 = ts3vc->joinChannel(1, QString());
				QObject::connect(reply2, &QCorReply::finished, [ts3vc, reply2, sendVideo]()
				{
					reply2->deleteLater();
					HL_DEBUG(HL, QString(reply2->frame()->data()).toStdString());

					// Send video.
					if (sendVideo)
					{
						auto baseDir = QDir("D:\\Temp\\camera");
						static auto frameNo = 0;

						//auto videoWidget = new VideoWidget();
						//videoWidget->resize(IFVS_CLIENT_VIDEO_SIZE);
						//videoWidget->show();

						auto t = new QTimer();
						t->setInterval(1000 / 15);
						t->start();
						QObject::connect(t, &QTimer::timeout, [t, baseDir, ts3vc]()
						{
							auto path = baseDir.filePath(QString("frame-%1.png").arg(frameNo++, 5, 10, QChar('0')));
							QImage image;
							image.load(path, "PNG");
							if (image.isNull())
							{
								frameNo = 0;
								return;
							}
							image.scaled(IFVS_CLIENT_VIDEO_SIZE);
							//videoWidget->setFrame(image);
							if (ts3vc->isReadyForStreaming())
								ts3vc->sendVideoFrame(image);
						});
					}

				});
			});
		});

		// Disconnected.
		QObject::connect(ts3vc, &NetworkClient::disconnected, [ts3vc, &ts3vconns]()
		{
			ts3vconns.removeAll(ts3vc);
			ts3vc->deleteLater();
		});

	});

	return a.exec();
}*/

/*!
    It seems like that it doesn't work under Windows.
    TODO Test under Linux
*/
/*int runVideoRecorderTest(QApplication& a)
{
	a.setQuitOnLastWindowClosed(true);

	auto camera = new QCamera(QCameraInfo::defaultCamera());
	camera->start();

	auto recorder = new QMediaRecorder(camera);

	QVideoEncoderSettings videoSettings;
	//videoSettings.setCodec("video/mpeg2");
	videoSettings.setResolution(IFVS_CLIENT_VIDEO_SIZE);
	videoSettings.setQuality(QMultimedia::VeryHighQuality);
	videoSettings.setFrameRate(30.0);

	recorder->setVideoSettings(videoSettings);
	recorder->setOutputLocation(QUrl::fromLocalFile("D:\\Temp\\myvideo.mp4"));
	recorder->record();

	auto videoWidget = new QVideoWidget();
	camera->setViewfinder(videoWidget);
	videoWidget->show();
	videoWidget->resize(IFVS_CLIENT_VIDEO_SIZE);

	QObject::connect(recorder, static_cast<void(QMediaRecorder::*)(QMediaRecorder::Error)>(&QMediaRecorder::error), [camera, recorder](QMediaRecorder::Error error)
	{
		qDebug() << QString("Error: %1").arg(recorder->errorString());
	});

	QTimer timer;
	timer.setSingleShot(true);
	timer.start(3000);
	QObject::connect(&timer, &QTimer::timeout, [camera, recorder]()
	{
		recorder->stop();
		camera->stop();
	});

	return a.exec();
}*/

/*!
    Records a video from QCamera and saves each frame as a *.png file to disk.
    Names the files by incrementing names (e.g.: frame-00001.png, frame-00002.png, ...)
*/
/*int runRecordPlainCameraImages(QApplication& a)
{
	a.setQuitOnLastWindowClosed(true);

	auto baseDir = QDir("D:\\Temp\\camera");
	auto frameNo = 0;

	auto camera = new QCamera(QCameraInfo::defaultCamera());
	camera->start();

	auto grabber = new CameraFrameGrabber(nullptr);
	camera->setViewfinder(grabber);

	auto videoWidget = new VideoWidget();
	videoWidget->resize(IFVS_CLIENT_VIDEO_SIZE);
	videoWidget->show();

	QObject::connect(grabber, &CameraFrameGrabber::newQImage, [baseDir, &frameNo, videoWidget](const QImage & image)
	{
		auto path = baseDir.filePath(QString("frame-%1.png").arg(frameNo++, 5, 10, QChar('0')));
		image.save(path, "PNG");
		videoWidget->setFrame(image);
	});

	return a.exec();
}*/

/*!
    Plays a video from plain frames read from *.png files on disk.
*/
/*int runPlayPlainCameraImages(QApplication& a)
{
	a.setQuitOnLastWindowClosed(true);

	auto baseDir = QDir("D:\\Temp\\camera");
	auto frameNo = 0;

	auto videoWidget = new VideoWidget();
	videoWidget->resize(IFVS_CLIENT_VIDEO_SIZE);
	videoWidget->show();

	QTimer t;
	t.setInterval(1000 / 15);
	t.start();
	QObject::connect(&t, &QTimer::timeout, [&t, &baseDir, &frameNo, videoWidget]()
	{
		auto path = baseDir.filePath(QString("frame-%1.png").arg(frameNo++, 5, 10, QChar('0')));
		QImage image;
		image.load(path, "PNG");
		if (image.isNull())
		{
			frameNo = 0;
			return;
		}
		videoWidget->setFrame(image);
	});

	return a.exec();
}*/

/*int runVideoPlayerTest(QApplication& a)
{
	a.setQuitOnLastWindowClosed(true);

	auto videoWidget = new QVideoWidget();
	videoWidget->show();

	auto player = new QMediaPlayer();
	player->setMedia(QUrl::fromLocalFile("D:/Temp/video.mp4"));
	player->setVolume(50);
	player->setVideoOutput(videoWidget);
	player->play();

	return a.exec();
}*/

/*int runRegisterUriHandler(QApplication& a)
{
	// TODO Run with elevated privileges!
	ELWS::unregisterURISchemeHandler("ts3video");
	ELWS::registerURISchemeHandler("ts3video", QString(), QString(), QString("--uri \"%1\""));
	return 0;
}*/

/*int runUnregisterUriHandler(QApplication& a)
{
	// TODO Run with elevated privileges!
	ELWS::unregisterURISchemeHandler("ts3video");
	return 0;
}*/




	// Run a specific mode.
	const auto mode = ELWS::getArgsValue("--mode").toString();
	if (mode == QString("test-multi-client"))
	{
		return runTestClient(a);
	}
	else if (mode == QString("test-gui4"))
	{
		return runTileViewTest(a);
	}
	else if (mode == QString("install-uri-handler"))
	{
		return runRegisterUriHandler(a);
	}
	else if (mode == QString("uninstall-uri-handler"))
	{
		return runUnregisterUriHandler(a);
	}
	else if (mode == QString("record-video"))
	{
		return runVideoRecorderTest(a);
	}
	else if (mode == QString("play-video"))
	{
		return runVideoPlayerTest(a);
	}
	else if (mode == QString("record-plain-video"))
	{
		return runRecordPlainCameraImages(a);
	}
	else if (mode == QString("play-plain-video"))
	{
		return runPlayPlainCameraImages(a);
	}

////////////////////////////

  QObject::connect(cameraButton, &QPushButton::toggled, [this, cameraButton](bool checked) {
    HL_DEBUG(HL, QString("Toggle camera (on=%1)").arg(checked).toStdString());
    cameraButton->setEnabled(false);
    QCorReply *reply = nullptr;
    if (checked)
      reply = d->logic->networkClient()->enableVideoStream();
    else
      reply = d->logic->networkClient()->disableVideoStream();
    if (!reply)
      return;
    QObject::connect(reply, &QCorReply::finished, [reply, cameraButton]()
    {
      cameraButton->setEnabled(true);
      reply->deleteLater();
    });
  });