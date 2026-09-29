\# Idea for a Drum VST3



\- modulator osc frequency modulates carrier osc

\- carrier osc into drive

\- drive output and noise transient mixed

\- mixer into filter

\- filter into ring mod

\- ring mod into grit fx

\- grit fx into amp



Please give little oscilloscopes in each block to see what's happening.



\## Controls



all continuously variable controls should be able to be right clicked to edit the value in a little hovering text box



1\. Carrier Controls

&#x20;   1. pitch tracking style, selector

&#x20;       - fixed frequency

&#x20;       - fixed pitch

&#x20;       - midi pitch

&#x20;       - fixed noise

&#x20;   1. pitch / frequency value, continuously variable

&#x20;       - fixed frequency: 20 Hz to 20 kHz

&#x20;       - fixed pitch: midi note 0 to 127

&#x20;           - show notes names as well, like: "C4 \[60]"

&#x20;       - midi pitch: note offset from -60 to +60

&#x20;       - fixed noise: sample and hold rate 0.1 Hz to 5 kHz

&#x20;   1. shape, continuously variable

&#x20;       - waveform (this should crossfade between the shapes)

&#x20;           - sine (at 0% knob range)

&#x20;           - triangle

&#x20;           - saw

&#x20;           - square (at 50% knob range)

&#x20;           - pwm 0% (at 100% knob range)

&#x20;       - fixed noise LPF

&#x20;           - 20 Hz to 20 kHz

&#x20;   1. level, continuously variable

&#x20;       - 0% to 100% to 400%

1\. Modulator Controls

&#x20;   1. type, selector

&#x20;       - fixed oscillator

&#x20;       - following oscillator

&#x20;       - fm operator

&#x20;       - fixed sine\*noise (ring mod'd together)

&#x20;       - following sine\*noise (ring mod'd together)

&#x20;       - fixed sample and hold noise

&#x20;       - fast decay envelope

&#x20;       - slow decay envelope

&#x20;   1. shape, continuously variable

&#x20;       - oscillator waveform (this should crossfade between the shapes)

&#x20;           - sine

&#x20;           - triangle

&#x20;           - saw

&#x20;           - square (at 50% knob range)

&#x20;           - pwm 0%

&#x20;       - noise, sample and hold rate

&#x20;           - 0.1 Hz to 20 kHz

&#x20;       - envelope, slope

&#x20;           - exponential to linear to logarithmic

&#x20;   1. depth, continuously variable

&#x20;       - -100% to 0 to +100%

&#x20;   1. speed, continuously variable

&#x20;       - fixed frequency: 0.1 Hz to 5 kHz

&#x20;       - following offset: -64 to 0 to +64 midi notes

&#x20;       - fm ratio: 1:32.0 to 1.0:1.0 to 32.0:1

&#x20;       - time: 10 ms to 333ms to 5 seconds

&#x20;       - time: 100 ms to 5 seconds to 60 seconds

1\. Drive Controls

&#x20;   1. type, selector

&#x20;       - off

&#x20;       - saturation

&#x20;       - clipper

&#x20;       - wave folder

&#x20;   1. drive, continuously variable

&#x20;       - saturation: -6dB to 0dB to +24dB

&#x20;       - clipper: -96dB to 0dB

&#x20;       - folder: -INFdB to 0dB

&#x20;   1. bias, continuously variable

&#x20;       - dc offset: -1 to 0 to +1

&#x20;   1. filter, continuously variable

&#x20;       - dj style filter

&#x20;           - 0% to 49%

&#x20;               - LPF: 20 Hz to 20 kHz

&#x20;               - slope: steep to flat

&#x20;           - 50%: no filter

&#x20;           - 51% to 100%

&#x20;               - HPF: 20 Hz to 20 kHz

&#x20;               - slope: flat to steep

1\. Noise Transient Controls

&#x20;   1. sample and hold rate, continuously variable

&#x20;       - 0.1 Hz to 20 kHz

&#x20;   1. filter, continuously variable

&#x20;       - dj style filter

&#x20;           - 0% to 49%

&#x20;               - LPF: 20 Hz to 20 kHz

&#x20;               - slope: steep to flat

&#x20;           - 50%: no filter

&#x20;           - 51% to 100%

&#x20;               - HPF: 20 Hz to 20 kHz

&#x20;               - slope: flat to steep

&#x20;   1. level, continuously variable

&#x20;       - 0% to 100%

&#x20;   1. decay, continuously variable

&#x20;       - 10 ms to 333ms to 5 seconds

1\. Filter Controls

&#x20;   1. type, selector

&#x20;       - no rez, LPF

&#x20;       - no rez, BPF

&#x20;       - no rez, HPF

&#x20;       - no rez, Notch

&#x20;       - rezzy, LPF

&#x20;       - rezzy, BPF

&#x20;       - rezzy, HPF

&#x20;       - rezzy, Notch

&#x20;       - comb

&#x20;       - APF (Disperser)

&#x20;   1. cutoff, continuously variable

&#x20;       - 20 Hz to 20 kHz

&#x20;   1. depth, continuously variable

&#x20;       - decay envelope

&#x20;           -100% to 0% to +100%

&#x20;       - resonance on comb

&#x20;           - -100% to 0% to +100%

&#x20;       - resonance on APFs

&#x20;           - -100% to 0% to +100%

&#x20;   1. decay, continuously variable

&#x20;       - no rez: 100 ms to 5 seconds to 60 seconds

&#x20;       - rezzy: 10 ms to 333ms to 5 seconds

&#x20;       - dampening on comb

&#x20;           - 0.1 Hz to 20 kHz

&#x20;       - simultaneous APFs

&#x20;           - 0 to 32

1\. RingMod Controls

&#x20;   1. waveform, continuously variable (this should crossfade between the shapes)

&#x20;       - sine (at 0% knob range)

&#x20;       - triangle

&#x20;       - saw

&#x20;       - square (at 50% knob range)

&#x20;       - pwm 0% (at 100% knob range)

&#x20;   1. rate, continuously variable

&#x20;       - 0.1 Hz to 5 kHz

&#x20;   1. amount, continuously variable

&#x20;       - 0% to 100%

&#x20;   1. width, continuously variable

&#x20;       - -100% to 0% to +100%

1\. Grit FX Controls

&#x20;   1. bit rate reduction, continuously variable

&#x20;       - 1.0 bit to 16.0 bit

&#x20;   1. sampe rate reduction, continuously variable

&#x20;       - 20 Hz to 20 kHz

1\. Frequency Shifter Controls

&#x20;   1. shift, continuously variable (the range of shift should go from negative to 0 Hz to postive of the value of the range control)

&#x20;       - -X Hz to 0 Hz to +X Hz

&#x20;   1. range, continuously variable

&#x20;       - 0 Hz to 5 kHz

&#x20;   1. blend, continuously variable

&#x20;       - -100% to -50/-50 to 0% to +50/+50 to +100%

&#x20;   1. width, continuously variable

&#x20;       - -100% to 0% to +100%

1\. Amp Controls

&#x20;   1. pan, continuously variable

&#x20;       - 100% left to center to 100% right

&#x20;   1. level, continuously variable

&#x20;       - 0% to 100% to 400%

&#x20;   1. drive, continuously variable

&#x20;       - -6dB to 0dB to +24dB

&#x20;   1. low boost, continuously variable

&#x20;       - 0dB to +24dB

1\. Amp Envelope Controls

&#x20;   1. type, selector

&#x20;       - fast decay envelope

&#x20;       - slow decay envelope

&#x20;   1. claps, continuously variable

&#x20;       - claps: 1 to 16

&#x20;   1. shape, continuously variable

&#x20;       - slope: exponential to linear to logarithmic

&#x20;   1. decay, continuously variable

&#x20;       - fast time: 10 ms to 333ms to 5 seconds

&#x20;       - slow time: 100 ms to 5 seconds to 60 seconds

