package com.insanefactory.cor;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by Manuel on 04.01.2015.
 */
public class CorParser {

    public interface ParserCallbacks {
        public int onFrameBegin();
        public int onFrameHeaderBegin();
        public int onFrameHeaderEnd();
        public int onFrameBodyBegin();
        public int onFrameBodyData();
        public int onFrameBodyEnd();
        public int onFrameEnd();
    }

    private enum ParserState {
        None,
        HeaderBegin,
        HeaderVersion,
        HeaderType,
        HeaderFlags,
        HeaderCorrelation,
        HeaderBodyLength,
        HeaderEnd,
        BodyBegin,
        BodyData,
        BodyEnd
    }

    private ParserCallbacks _callbacks;
    private ParserState _state;
    private CorFrame _frame;
    private ByteArrayOutputStream _frameBodyOutput;

    public CorParser(ParserCallbacks callbacks) {
        _callbacks = callbacks;
        _frameBodyOutput = new ByteArrayOutputStream();
        reset();
    }

    public void reset() {
        _state = ParserState.None;
        _frame = null;
        _frameBodyOutput.reset();
    }

    public CorFrame getCurrentFrame() {
        return _frame;
    }

    public void parse(InputStream in) throws IOException {
        DataInputStream dis = new DataInputStream(in); // Reads with big endian.
        while (true) {
            switch (_state) {
                case None:
                    _frame = new CorFrame();
                    _state = ParserState.HeaderBegin;
                    _callbacks.onFrameBegin();
                    break;
                case HeaderBegin:
                    _state = ParserState.HeaderVersion;
                    _callbacks.onFrameHeaderBegin();
                    break;
                case HeaderVersion:
                    _frame.version = dis.readShort();
                    _state = ParserState.HeaderType;
                    break;
                case HeaderType:
                    _frame.type = dis.readShort();
                    _state = ParserState.HeaderFlags;
                    break;
                case HeaderFlags:
                    _frame.flags = dis.readByte();
                    _state = ParserState.HeaderCorrelation;
                    break;
                case HeaderCorrelation:
                    _frame.correlation = dis.readInt();
                    _state = ParserState.HeaderBodyLength;
                    break;
                case HeaderBodyLength:
                    _frame.bodyLength = dis.readInt();
                    _state = ParserState.BodyBegin;
                    _callbacks.onFrameHeaderEnd();
                    break;
                case BodyBegin:
                    _state = ParserState.BodyData;
                    _callbacks.onFrameBodyBegin();
                    break;
                case BodyData:
                    while (_frameBodyOutput.size() < _frame.bodyLength) {
                        _frameBodyOutput.write(dis.readByte());
                        _callbacks.onFrameBodyData();
                    }
                    _state = ParserState.BodyEnd;
                    break;
                case BodyEnd:
                    _frame.body = _frameBodyOutput.toByteArray();
                    _callbacks.onFrameBodyEnd();
                    _callbacks.onFrameEnd();
                    reset();
                    break;
            }
        }
    }

}
