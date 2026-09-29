\# Idea for a Drum VST3



\- modulator osc frequency modulates carrier osc

\- carrier osc into drive

\- drive output and noise transient into mixer

\- mixer into filter

\- filter into ring mod

\- ring mod into frequency shifter

\- frequency shifter into grit fx

\- grit fx into comb filter

\- comb filter into disperser

\- disperser into amp



Please give little oscilloscopes in each block to see what's happening.



\## Controls



all continuously variable controls should be able to be right clicked to edit the value in a little hovering text box



for bipolar controls, have the UI show the fill on the knob starting from the default center position



for unipolar controls, have the UI show the fill on the knob starting from minimum



for all slope controls, show a little diagram of the current slope instead of the value (but can still be right clocked to edit the value)



for all waveform controls, show a little diagram of the current wave shape instead of the value (but can still be right clocked to edit the value)



1\. Carrier Controls

&#x20;   1. pitch tracking style, selector

&#x20;       - fixed frequency

&#x20;       - fixed pitch

&#x20;       - midi pitch (default)

&#x20;   1. pitch / frequency value, continuously variable

&#x20;       - fixed frequency: 20 Hz to 20 kHz (default 55 Hz)

&#x20;       - fixed pitch: midi note 0 to 127 (default "A1 \[33]")

&#x20;           - show notes names as well as midi note number: "C4 \[60]"

&#x20;       - midi pitch: note offset from -60 to +60 (default 0)

&#x20;   1. shape, continuously variable

&#x20;       - waveform (this should crossfade between the shapes)

&#x20;           - sine (at 0% knob range, the default)

&#x20;           - triangle (at 20% knob range)

&#x20;           - saw (at 40% knob range)

&#x20;           - square (at 60% knob range)

&#x20;           - pwm 0% (at 100% knob range)

&#x20;   1. drive, continuously variable

&#x20;       - -6dB to 0dB (at 50% knob) to +24dB

&#x20;       - default: 0dB

1\. Modulator Controls

&#x20;   1. type, selector

&#x20;       - fixed oscillator

&#x20;       - following oscillator

&#x20;       - fm operator (linear FM)

&#x20;       - fixed sine \* white noise (sine and noise ring mod'd together)

&#x20;       - following sine \* white noise (sine and noise ring mod'd together)

&#x20;       - fm operator sine \* white noise (sine and noise ring mod'd together, linear fm)

&#x20;       - sample and hold noise

&#x20;   1. shape, continuously variable

&#x20;       - oscillator waveform (this should crossfade between the shapes)

&#x20;           - sine (at 0% knob range, the default)

&#x20;           - triangle (at 20% knob range)

&#x20;           - saw (at 40% knob range)

&#x20;           - square (at 60% knob range)

&#x20;           - pwm 0% (at 100% knob range)

&#x20;       - white noise filter

&#x20;           - dj style filter

&#x20;               - 0% to 49%

&#x20;                   - LPF: 20 Hz to 20 kHz

&#x20;                   - slope: steep to flat

&#x20;               - 50%: no filter (the default)

&#x20;               - 51% to 100%

&#x20;                   - HPF: 20 Hz to 20 kHz

&#x20;                   - slope: flat to steep

&#x20;       - sample and hold noise rate

&#x20;           - 0.1 Hz to 20 kHz

&#x20;           - default: 20 kHz

&#x20;   1. depth, continuously variable

&#x20;       - -200% to 0% to +200%

&#x20;       - default; 0%

&#x20;   1. speed, continuously variable

&#x20;       - fixed frequency: 0.1 Hz to 15 kHz (default 55 Hz)

&#x20;       - following offset: -64 to 0 to +64 midi notes (default 0)

&#x20;       - fm ratio: 1:32.0 to 1.0:1.0 to 32.0:1 (default 1.0:1.0)

&#x20;       - sample and hold rate: 0.1 Hz to 20 kHz (default: 20 kHz)

1\. Pitch Envelope Controls

&#x20;   1. oscillator, selector

&#x20;       - off (default)

&#x20;       - carrier

&#x20;       - modulator

&#x20;       - both

&#x20;   1. slope, continuously variable

&#x20;       - exponential to linear to logarithmic

&#x20;       - default: exponential

&#x20;   1. depth, continuously variable

&#x20;       - -5 Octaves to 0 to +5 Octaves

&#x20;       - default: 0

&#x20;   1. decay, continuously variable

&#x20;       - 5 ms (at 0%)

&#x20;       - 100 ms (at 25%)

&#x20;       - 1 second (at 50%)

&#x20;       - 5 seconds (at 75%)

&#x20;       - 60 seconds (at 100%)

&#x20;       - default: 333 ms

1\. Drive Controls

&#x20;   1. type, selector

&#x20;       - off (default)

&#x20;       - saturation

&#x20;       - wave folder

&#x20;   1. drive, continuously variable

&#x20;       - saturation: -6dB to 0dB to +24dB (default: 0dB)

&#x20;       - folder: 0 folds to 8 folds (default: 0)

&#x20;   1. bias, continuously variable

&#x20;       - dc offset: -1 to 0 to +1

&#x20;       - default: 0

&#x20;   1. post-filter, continuously variable

&#x20;       - dj style filter

&#x20;           - 0% to 49%

&#x20;               - LPF: 20 Hz to 20 kHz

&#x20;               - slope: steep to flat

&#x20;           - 50%: no filter (the default)

&#x20;           - 51% to 100%

&#x20;               - HPF: 20 Hz to 20 kHz

&#x20;               - slope: flat to steep

1\. Noise Transient Controls

&#x20;   1. sample and hold rate, continuously variable

&#x20;       - 0.1 Hz to 20 kHz

&#x20;       - default: 20 kHz

&#x20;   1. filter, continuously variable

&#x20;       - dj style filter

&#x20;           - 0% to 49%

&#x20;               - LPF: 20 Hz to 20 kHz

&#x20;               - slope: -24dB/oct to -6dB/oct

&#x20;           - 50%: no filter (the default)

&#x20;           - 51% to 100%

&#x20;               - HPF: 20 Hz to 20 kHz

&#x20;               - slope: -6dB/oct to -24dB/oct

&#x20;   1. drive, continuously variable

&#x20;       - -6dB to 0dB (at 50% knob) to +24dB

&#x20;       - default: 0dB

&#x20;   1. decay, continuously variable

&#x20;       - 1 ms (at 0%)

&#x20;       - 50 ms (at 25%)

&#x20;       - 1 second (at 50%)

&#x20;       - 5 seconds (at 75%)

&#x20;       - 60 seconds (at 100%)

&#x20;       - default: 100 ms

1\. Mixer Controls

&#x20;   1. limiter, selector

&#x20;       - off

&#x20;       - on (default)

&#x20;   1. carrier level (this is after the drive block), continuously variable

&#x20;       - 0% to 100%  (at 50% knob range) to 400%

&#x20;       - default: 100%

&#x20;   1. noise level, continuously variable

&#x20;       - 0% to 100%  (at 50% knob range) to 400%

&#x20;       - default: 0%

&#x20;   1. drive, continuously variable

&#x20;       - -6dB to 0dB (at 50% knob) to +24dB

&#x20;       - default: 0dB

1\. Filter Controls

&#x20;   1. type, selector

&#x20;       - off (default)

&#x20;       - LPF

&#x20;       - BPF

&#x20;       - HPF

&#x20;       - Notch

&#x20;   1. slope, continuously variable

&#x20;       - -6dB/oct to -24dB/oct (at 50%) to -96dB/oct

&#x20;       - default: -12dB/oct

&#x20;   1. cutoff, continuously variable

&#x20;       - 0.1 Hz to 20 kHz

&#x20;       - defaul: 20 kHz

&#x20;   1. resonance, continuously variable

&#x20;       - resonance on filter: 0% to 100%

&#x20;       - default: 0%

1\. Filter Envelope Controls

&#x20;   1. slope, continuously variable

&#x20;       - exponential to linear to logarithmic

&#x20;       - default: exponential

&#x20;   1. depth, continuously variable

&#x20;       - -100% to 0% to 100%

&#x20;       - default: 0%

&#x20;   1. decay, continuously variable

&#x20;       - 5 ms (at 0%)

&#x20;       - 100 ms (at 25%)

&#x20;       - 1 second (at 50%)

&#x20;       - 5 seconds (at 75%)

&#x20;       - 60 seconds (at 100%)

&#x20;       - default: 333 ms

&#x20;   1. pre-drive (for the filter, not the envelope), continuously variable

&#x20;       - -6dB to 0dB (at 50% knob) to +24dB

&#x20;       - default: 0dB

1\. RingMod Controls

&#x20;   1. waveform, continuously variable (this should crossfade between the shapes)

&#x20;       - sine (at 0% knob range, the default)

&#x20;       - triangle (at 20% knob range)

&#x20;       - saw (at 40% knob range)

&#x20;       - square (at 60% knob range)

&#x20;       - pwm 0% (at 100% knob range)

&#x20;   1. rate, continuously variable

&#x20;       - 0.1 Hz to 15 kHz

&#x20;       - default: 55 Hz

&#x20;   1. amount, continuously variable

&#x20;       - 0% to 100%

&#x20;       - default: 0%

&#x20;   1. width, continuously variable

&#x20;       - -100% to 0% to +100%

&#x20;       - default: 0%

1\. Frequency Shifter Controls

&#x20;   1. shift, continuously variable (the range of shift should go from negative X Hz to 0 Hz to postive X Hz, where X is the value of the range control)

&#x20;       - -X Hz to 0 Hz to +X Hz

&#x20;       - default: 0 Hz

&#x20;   1. range, continuously variable

&#x20;       - 0 Hz to 5 kHz

&#x20;       - default: 3 Hz

&#x20;   1. blend, continuously variable (-100% and +100% should both be fully frequency shifter; -50% and +50% should be 50% frequency shifter and 50% dry signal; 0% should be fully dry signal)

&#x20;       - -100% to -50% to 0% to +50% to +100%

&#x20;       - default: 0%

&#x20;   1. width, continuously variable

&#x20;       - -100% to 0% to +100%

&#x20;       - default: 0%

1\. Grit FX Controls

&#x20;   1. bit rate reduction, continuously variable

&#x20;       - 1.0 bit to 16.0 bit

&#x20;       - default: 16.0 Bit

&#x20;   1. sampe rate reduction, continuously variable

&#x20;       - 20 Hz to 20 kHz

&#x20;       - default: 20 kHz

&#x20;   1. low boost, continuously variable

&#x20;       - 0dB to +24dB

&#x20;       - default: 0dB

&#x20;   1. high boost, continuously variable

&#x20;       - 0dB to +24dB

&#x20;       - default: 0dB

1\. Disperser Controls

&#x20;   1. type, selector

&#x20;       - off (default)

&#x20;       - on

&#x20;   1. amount, continuously variable

&#x20;       - series of all-pass filters

&#x20;           - 0 to 32

&#x20;           - default: 4

&#x20;   1. cutoff, continuously variable

&#x20;       - 0.1 Hz to 20 kHz

&#x20;       - defaul: 20 kHz

&#x20;   1. resonance, continuously variable

&#x20;       - -100% to 0% to +100%

&#x20;       - default: 0%

1\. Comb Filter Controls

&#x20;   1. type, selector

&#x20;       - off (default)

&#x20;       - on

&#x20;   1. dampening, continuously variable

&#x20;       - 0.1 Hz to 20 kHz

&#x20;       - default: 20 kHz

&#x20;   1. cutoff, continuously variable

&#x20;       - 0.1 Hz to 20 kHz

&#x20;       - defaul: 20 kHz

&#x20;   1. resonance, continuously variable

&#x20;       - -100% to 0% to +100%

&#x20;       - default: 0%

1\. Amp Controls

&#x20;   1. limiter, selector

&#x20;       - off

&#x20;       - on (default)

&#x20;   1. pan, continuously variable

&#x20;       - 100% left to center to 100% right

&#x20;       - default: center

&#x20;   1. level, continuously variable

&#x20;       - 0% to 100% to 400%

&#x20;       - default: 100%

&#x20;   1. drive, continuously variable

&#x20;       - -6dB to 0dB (at 50% knob) to +24dB

&#x20;       - default: 0dB

1\. Amp Envelope Controls

&#x20;   1. claps, continuously variable

&#x20;       - claps: 0 to 32

&#x20;       - default: 0

&#x20;   1. clap speed, continuously variable

&#x20;       - decay time per clap: 1 ms to 15 ms

&#x20;       - default: 3ms

&#x20;   1. slope, continuously variable

&#x20;       - exponential to linear to logarithmic

&#x20;       - default: exponential

&#x20;   1. decay, continuously variable

&#x20;       - 5 ms (at 0%)

&#x20;       - 100 ms (at 25%)

&#x20;       - 1 second (at 50%)

&#x20;       - 5 seconds (at 75%)

&#x20;       - 60 seconds (at 100%)

&#x20;       - default: 333 ms

1\. Velocity Controls

&#x20;   1. slope, continuously variable (sets the slope of the velocity curve)

&#x20;       - exponential to linear to logarithmic

&#x20;       - default: linear

&#x20;   1. decay, continuously variable (adds to all decay times)

&#x20;       - -100% 0% to 100%

&#x20;       - default: 0%

&#x20;   1. depth, continuously variable (adds to all envelope depths)

&#x20;       - -100% to 0% to +100%

&#x20;       - default: 0%

&#x20;   1. volume, continuously variable (sets the minimum output volume for lowest velocity)

&#x20;       - 0% (no change to volume) to -100% (lowest velocity is quietest)

&#x20;       - default: 0%

