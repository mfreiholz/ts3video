package com.insanefactory.cor;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.util.concurrent.BlockingQueue;

/**
 * Created by Manuel on 03.01.2015.
 */
public class CorConnection {
    protected Socket _socket;
    protected Thread _inputThread;
    protected Thread _outputThread;

    public void connectToHost(InetAddress address, int port) throws IOException {
        _socket = new Socket(address, port);
        _socket.setKeepAlive(true);
        _socket.setTcpNoDelay(true);
        _inputThread = new Thread(new InputHandler(this));
        _inputThread.start();
        _outputThread = new Thread(new OutputHandler(this));
        _outputThread.start();
    }

    public void connectWith(Socket socket) throws IOException {
        _socket = socket;
        _socket.setKeepAlive(true);
        _socket.setTcpNoDelay(true);
        new Thread(new InputHandler(this)).start();
        new Thread(new OutputHandler(this)).start();
    }

    public void shutdown() throws IOException {
        if (_socket != null) {
            _socket.close();
            _socket = null;
        }
    }

    public void exec() throws InterruptedException {
        _inputThread.wait();
        _outputThread.wait();
    }

    /**
     *
     */
    private class InputHandler implements Runnable, CorParser.ParserCallbacks {
        private CorConnection _conn;
        private CorParser _parser;

        public InputHandler(CorConnection conn) {
            _conn = conn;
            _parser = new CorParser(this);
        }

        @Override
        public void run() {
            try {
                InputStream in = _conn._socket.getInputStream();
                _parser.parse(in);
            } catch (IOException ioe) {
                // Close the connection on error during parsing.
                try {
                    _conn.shutdown();
                } catch (IOException e) {}
            }
        }

        @Override
        public int onFrameBegin() {
            return 0;
        }

        @Override
        public int onFrameHeaderBegin() {
            return 0;
        }

        @Override
        public int onFrameHeaderEnd() {
            return 0;
        }

        @Override
        public int onFrameBodyBegin() {
            return 0;
        }

        @Override
        public int onFrameBodyData() {
            return 0;
        }

        @Override
        public int onFrameBodyEnd() {
            return 0;
        }

        @Override
        public int onFrameEnd() {
            CorFrame frame = _parser.getCurrentFrame();
            System.out.println("New frame: " + frame.toString());
            return 0;
        }
    }

    /**
     *
     */
    private class OutputHandler implements Runnable {
        private CorConnection _conn;
        private BlockingQueue _queue;

        public OutputHandler(CorConnection conn) {
            _conn = conn;
        }

        @Override
        public void run() {
        }
    }

    public interface Callbacks {
        void onNewFrame(CorFrame frame);
        void onDisconnected();
    }

}
