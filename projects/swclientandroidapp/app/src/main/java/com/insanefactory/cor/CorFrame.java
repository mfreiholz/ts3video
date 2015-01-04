package com.insanefactory.cor;

/**
 * Created by Manuel on 04.01.2015.
 */
public class CorFrame {
    public short version;
    public short type;
    public byte flags;
    public int correlation;
    public int bodyLength;
    public byte[] body;

    public String toString() {
        return "[" +
                "version=" + String.valueOf(version) + "; " +
                "type=" + String.valueOf(type) + "; " +
                "flags=" + String.valueOf(flags) + "; " +
                "correlation=" + String.valueOf(correlation) + "; " +
                "bodyLength=" + String.valueOf(bodyLength) + "; " +
                "body=" + new String(body) +
                "]";
    }
}
