##
# @file ranging.py
#
# @brief Ranging Eval Application Python interface
#
# $Id: ranging.py 34336 2013-02-22 09:00:03Z sschneid $
#
# @author    Atmel Corporation: http://www.atmel.com
# @author    Support email: avr@atmel.com
#
#
# Copyright (c) 2010, Atmel Corporation All rights reserved.
#
# Licensed under Atmel's Limited License Agreement --> EULA.txt
#
"""
Ranging Evaluation Terminal Application - V %s

  Usage:
    python -i ranging.py [OPTIONS] [script [script [...]]]

  System prerequisites:
    Nodes with preprogrammed address settings

  Options:
    -p <PORT>
       Name of the port to be opened.
       If this option is not present, the application
       prompts for entering a port name.
    -h
       Print help and exit.
    -V
       Print version number and exit.
    -v
       Increase python verbose level.

    script
        A python script, that can call the following commands.

  Commands:
    For details type help(<command>).

    help(...)
        Print a help message.

    configure( ... )
        Configure device parameters.

    run(...)
        Measure a distance.

    measdist(...)
        Run a measurement between a tag and a group of anchors.

    rangefinder(...)
        Starts permanent distance measurements between tag and anchor.

    reset()
        Reset the firmware of the board.

"""
# === modules =================================================================
import sys, traceback
import serial, threading, time
import getopt, pprint
try:
    import pydoc
except:
    print "pydoc not found"

#=== global variables =========================================================
VERSION = "1.1.7"
VERBOSE = 0
PORT    = None

# === classes =================================================================

## Class for controlling a ranging device.
class BasicDevice:
    ## Device classifier
    TYPE = "BasicDevice"
    ## measurement timeout value
    TMO = 2
    ## Error Code
    ERROR = 0
    ## data buffer for received bytes
    DATABUF = ""

    ## Constructor.
    def __init__(self, port = 'com1', baud = 38400):
        self.port           = port
        self.baud           = baud
        self.sport          = serial.Serial(self.port, self.baud, timeout=0)
        self.py_verbose     = 1
        self.resultbuffer   = []
        self.resultEvent    = threading.Event()
        self.execute_rx_thread = True
        self.rxThread = threading.Thread(target = self._rx_thread_)
        self.rxThread.setDaemon(1)
        self.rxThread.setName("RX[%s]" % self.sport.portstr)
        self.rxThread.start()
        self.mode = None
        self.param = {}
        self.no_of_measurements = 0
        self.results_antenna_div = []
        self.setparam(FilteringlengthduringcontinuousRanging=1) # disable rangefinder()

    ## Reset the firmware to the factory defaults.
    def reset(self):
        self.sport.write("F\n")

    ## Set the measurement parameters.
    def setparam(self,
                 Channel=None,
                 OwnShortAddress=None,
                 InitiatorShortAddressforRemoteRanging=None,
                 InitiatorLongAddressforRemoteRanging=None,
                 ReflectorShortAddress=None,
                 ReflectorLongAddress=None,
                 PAN_Id=None,
                 RangingAddressingScheme=None,
                 CoordinatorAddressingMode=None,
                 FilteringmethodforcontinuousRanging=None,
                 FilteringlengthduringcontinuousRanging=1,
                 DefaultAntenna=None,
                 AntennaDiversity=None,
                 ProvideallMeasurementResults=None,
                 FrequencyStart=None,
                 FrequencyStep=None,
                 FrequencyStop=None,
                 Verbose=None,
                 TxPowerduringRanging=None,
                 ProvideRangingTxPowerfornextRanging=None,
                 ApplyMinimumThresholdduringweightedDistanceCalc=None):
        ## Investigate present node setting
        tmp = self.getparam()
        cmd = ""

        ## FilteringlengthduringcontinuousRanging is always set to 1
        if tmp['FilteringlengthduringcontinuousRanging'] != FilteringlengthduringcontinuousRanging:
            number_cycles = 1
            cmd += "n%d\n" % number_cycles

        ## Communication parameters
        if Channel != None:
            if tmp['Channel'] != Channel:
                cmd += "c%d\n" % Channel
        if OwnShortAddress != None:
            if tmp['OwnShortAddress'] != OwnShortAddress:
                cmd += "o%d\n" % OwnShortAddress
        if InitiatorShortAddressforRemoteRanging != None:
            if tmp['InitiatorShortAddressforRemoteRanging'] != InitiatorShortAddressforRemoteRanging:
                cmd += "i%d\n" % InitiatorShortAddressforRemoteRanging
        if InitiatorLongAddressforRemoteRanging != None:
            if tmp['InitiatorLongAddressforRemoteRanging'] != InitiatorLongAddressforRemoteRanging:
                cmd += "I%d\n" % InitiatorLongAddressforRemoteRanging
        if ReflectorShortAddress != None:
            if tmp['ReflectorShortAddress'] != ReflectorShortAddress:
                cmd += "r%d\n" % ReflectorShortAddress
        if ReflectorLongAddress != None:
            if tmp['ReflectorLongAddress'] != ReflectorLongAddress:
                cmd += "R%d\n" % ReflectorLongAddress
        if PAN_Id != None:
            if tmp['PAN_Id'] != PAN_Id:
                cmd += "P%d\n" % PAN_Id
        if RangingAddressingScheme != None:
            if tmp['RangingAddressingScheme'] != RangingAddressingScheme:
                cmd += "s%d\n" % RangingAddressingScheme
        if CoordinatorAddressingMode != None:
            if tmp['CoordinatorAddressingMode'] != CoordinatorAddressingMode:
                cmd += "g%d\n" % CoordinatorAddressingMode
        if FilteringmethodforcontinuousRanging != None:
            if tmp['FilteringmethodforcontinuousRanging'] != FilteringmethodforcontinuousRanging:
                cmd += "f%d\n" % FilteringmethodforcontinuousRanging

        # Ranging parameters
        if DefaultAntenna != None:
            if tmp['DefaultAntenna'] != DefaultAntenna:
                cmd += "d%d\n" % DefaultAntenna
        if AntennaDiversity != None:
            if tmp['AntennaDiversity'] != AntennaDiversity:
                cmd += "a%d\n" % AntennaDiversity
        if ProvideallMeasurementResults != None:
            if tmp['ProvideallMeasurementResults'] != ProvideallMeasurementResults:
                cmd += "e%d\n" % ProvideallMeasurementResults
        if FrequencyStart != None:
            if tmp['FrequencyStart'] != FrequencyStart:
                cmd += "1%d\n" % FrequencyStart
        if FrequencyStep != None:
            if tmp['FrequencyStep'] != FrequencyStep:
                cmd += "2%d\n" % FrequencyStep
        if FrequencyStop != None:
            if tmp['FrequencyStop'] != FrequencyStop:
                cmd += "3%d\n" % FrequencyStop
        if ApplyMinimumThresholdduringweightedDistanceCalc != None:
            if tmp['ApplyMinimumThresholdduringweightedDistanceCalc'] != ApplyMinimumThresholdduringweightedDistanceCalc:
                cmd += "w%d\n" % ApplyMinimumThresholdduringweightedDistanceCalc

        # Misc. parameters
        if TxPowerduringRanging != None:
            if tmp['TxPowerduringRanging'] != TxPowerduringRanging:
                cmd += "t%d\n" % TxPowerduringRanging
        if ProvideRangingTxPowerfornextRanging != None:
            if tmp['ProvideRangingTxPowerfornextRanging'] != ProvideRangingTxPowerfornextRanging:
                cmd += "T%d\n" % ProvideRangingTxPowerfornextRanging

        if Verbose != None:
            if tmp['Verbose'] != Verbose:
                if Verbose == 1:
                    cmd += "v1\n"
                else:
                    cmd += "v0\n"
                self.py_verbose = Verbose

        self.sport.write(cmd)

    ## Return a dictionary with the current parameter settings.
    def getparam(self):
        self.resultEvent.clear()
        self.sport.write("p")
        self.resultEvent.wait(self.TMO)
        return self.param

    ## Return the result dictionary.
    def get_results(self):
        ret = {}
        if self.resultbuffer == []:
            if self.ERROR == 0:
                ret = {
                    'dist' : self.result[0],
                    'dqf'  : self.result[1],
                    'initiator' : self.result[2],
                    'reflector' : self.result[3]
                }
            else:
                ret = {
                    'dist' : self.result[0],
                    'dqf'  : self.result[1],
                    'initiator' : self.result[2],
                    'reflector' : self.result[3],
                    'error' : self.result[4]
                }
        return ret

    ##
    # Run a measurement.
    #
    # @param reflector
    #           16 bit reflector address
    # @param initiator
    #           16 bit inititator address
    #
    def run(self, reflector=None, initiator=None):
        self.ERROR = 0
        self.resultEvent.clear()
        self.resultbuffer = []
        if initiator != None and reflector != None:
            # Set Reflector Short address
            self.sport.write("r%d\n" % reflector)
            if initiator == self.param['OwnShortAddress']:
                self.sport.write("m")
            else:
                # Remote ranging
                # Set Initiator Short Address for Remote Ranging
                self.sport.write("i%d\n" % initiator)
                self.sport.write("M")
        else:
            if reflector!=None:
                # Set Reflector Short address
                self.sport.write("r%d\n" % reflector)
            self.sport.write("m")
        self.resultEvent.wait(self._wait_for_data_())
        if self.resultEvent.isSet() == False:
            # Timeout condition
            self.ERROR = 255    # 255: Timeout
            if VERBOSE:
                print "TIMEOUT"
            ret = {
                'dist' : -1,
                'dqf'  : 0,
                'initiator' : initiator,
                'reflector' : reflector,
                'error' : 255
                }
        else:
            # No timeout
            ret = self.get_results()
            if self.ERROR != 0:
                if VERBOSE:
                    print "ERROR[%d]" % self.ERROR
            if self.no_of_measurements > 1:
                dummy = self.results_antenna_div
                if VERBOSE:
                    print self.results_antenna_div
        return ret

    ## Adjust delay .
    def _wait_for_data_(self):
        self.TMO
        if (self.param['FrequencyStep'] == 2):
            fstep = 2
        elif (self.param['FrequencyStep'] == 3):
            fstep = 4
        else:
            fstep = 1
        wait_for_data_delay  = self.TMO
        wait_for_data_delay += 0.16*(((self.param['FrequencyStop']-self.param['FrequencyStart'])/fstep)+1)*self.param['Verbose']
        return wait_for_data_delay

    ## Serial line RX handler method.
    def _rx_thread_(self):
        textmode = True
        dowait = True
        self.bresults = []
        while self.execute_rx_thread:
            try:
                self._serial_get_data_(dowait)
                textmode, dowait = self._process_rx_line_()
            except:
                sys.stderr.write("ERROR:rxthread:%s:%s\n" % \
                    (self.sport.portstr, sys.exc_info()[1]))
                traceback.print_exc()
                print "Zzzzzzzzzzzz...."
                time.sleep(2)

    ## Release Serial line RX handler.
    def enable_rx_thread(self):
        self.execute_rx_thread = True

    ## Stop Serial line RX handler.
    def disable_rx_thread(self):
        self.execute_rx_thread = False

    ## Read serial buffer.
    def _serial_get_data_(self, dowait = False):
        if dowait:
            # needed for windows, otherwise the RX thread
            # blocks the whole system
            time.sleep(0.05)
            # swap timeout to block the thread
            self.DATABUF += self.sport.read(1)
        n = self.sport.inWaiting()
        if n > 0:
            self.DATABUF += self.sport.read(n)

    ## Extract a line from the data collector.
    def _serial_getline_(self):
        ret = None
        try:
            eolidx = self.DATABUF.find('\n') + 1
            ret = self.DATABUF[:eolidx].rstrip()
            if len(ret) == 0:
                ret = None
            self.DATABUF = self.DATABUF[eolidx:]
        except ValueError:
            pass
        return ret

    ## Extract a block of bytes from the data collector.
    def _serial_getbytes_(self, nbbytes):
        ret = None
        try:
            x = self.DATABUF[nbbytes+1] # throw IndexError if buffer is too short.
            ret = self.DATABUF[:nbbytes]
            self.DATABUF = self.DATABUF[nbbytes:]
        except IndexError:
            pass
        return ret

    ## Interpret lines modespecific.
    def _decode_line_(self,line):
        if self.mode == None:
            return
        elif self.mode == "end":
            self.resultEvent.set()
            self.mode = None
            # set event
        elif self.mode == "param":
            if line.find(":") > 0:
                if line.find("=") > 0:
                    tmp = line.split()
                    colon_idx = tmp.index(":")
                    equal_idx = tmp.index("=")
                    name = ''.join(tmp[(colon_idx + 1): equal_idx])
                    value = tmp[equal_idx + 1]
                    if (value == 'Average'):
                        value = '0'
                    elif (value == 'Median'):
                        value = '1'
                    elif (value == 'Min.'):
                        argument = ""
                        for element in tmp[(equal_idx + 1):]:
                            argument += element
                        if (argument == 'Min.ofdistanceandDQF'):
                            value = '2'
                        elif (argument == 'Min.ofdistanceandDQFconsiderungvariance'):
                            value = '3'
                        else:
                            value = '0'
                    elif (value == 'Max.'):
                        value = '4'
                    self.param[name] = eval(value)

    ##
    # Process a text line received from the serial interface.
    #
    # @return (textmode, dowait)
    #       - textmode: True if parsing should proceed in text mode, otherwise False.
    #       - dowait:   if True do a blocking read from serial, no line was available
    def _process_rx_line_(self):
        ret = True
        line = self._serial_getline_()
        if line:
            if self.py_verbose >= 2:
                sys.stdout.write("    >" + line+"\n")
                sys.stdout.flush()
                # do keyword spotting
            if line.find('[ERROR]') == 0:
                self.no_of_measurements = 0
                self.ERROR = eval(line.split()[-1])
                self.result = map(eval, line.split()[1:])
            elif line.find('[RESULT]') == 0:
                self.no_of_measurements = 0
                self.results_antenna_div = []
                self.result = map(eval, line.split()[1:])
            elif line.find('[DONE]') == 0:
                self.resultEvent.set()
            elif line.find('[PAIR_NO_') == 0:   # Antenna Diversity
                self.no_of_measurements += 1
                self.results_antenna_div.append(map(eval, line.split()[1:]))
            elif line.find('[PARAM]') == 0:
                self.mode = "param"
            elif line.find('[PARAM_END]') == 0:
                self.mode = "end"
            self._decode_line_(line)

        return (ret, line==None)

# === implementation ==========================================================

## Configure the ranging device parameters.
#
# @param cfg
#           a dictionary with parameters.
# @param Channel
#           Used communication channel (11-26)
# @param OwnShortAddress
#           Short address of local node (16 bit number)
# @param InitiatorShortAddressforRemoteRanging
#           Used short address of Initiator in case local node is Coordinator for remote ranging (16 bit number)
# @param InitiatorLongAddressforRemoteRanging
#           Used long address of Initiator in case local node is Coordinator for remote ranging (64 bit number)
# @param ReflectorShortAddress
#           Short address of reflector node (16 bit number)
# @param ReflectorLongAddress
#           Long address of reflector node (64 bit number)
# @param PAN_Id
#           PAN_Id of ranging network (16 bit number)
# @param RangingAddressingScheme
#           Ranging Addressing Scheme for local ranging
#           0 - Initiator short address, Reflector short address
#           1 - Initiator short address, Reflector long address
#           2 - Initiator long address, Reflector short address
#           3 - Initiator long address, Reflector long address
# @param CoordinatorAddressingMode
#           Coordinator Addressing Mode used if node is Coordinator in remote ranging
#           2 - Short address
#           3 - Long address
# @param DefaultAntenna
#           Used default antenna of local node (in case antenna diversity if off) (0, 1)
# @param AntennaDiversity
#           Enabling of antenna diversity of local node (0, 1)
# @param ProvideallMeasurementResults
#           Enabling of provisioning of all measured distances and DQFs in case of antenna diversity (0, 1)
# @param FrequencyStart
#           Set PMU start frequency (2403 ... 2481 MHz)
# @param FrequencyStep
#           Set PMU step size (0 = 0.5 MHz, 1 = 1.0 MHz, 2 = 2.0 MHz, 3 = 4.0 MHz)
# @param FrequencyStop
#           Set PMU stop frequency (2403 ... 2481 MHz)
# @param ApplyMinimumThresholdduringweightedDistanceCalc
#           Disable/enable wieghted distance minimum search (0, 1)
# @param Verbose
#           Set terminal verbosity (0, 1)
# @param TxPowerduringRanging
#           Apply Tx Power during Ranging in dBm
# @param ProvideRangingTxPowerfornextRanging
#           Force Tx Power setting for next Ranging at peer nodes (0, 1)
#
# Usage:
#
# * print current configuration / parameter names
# >>> pp(configure())
# >>> pp(configure().keys())
#
# * set FrequencyStop value
# >>> pp(configure(FrequencyStop = 2481))
#
# * save current configuration in a dictionary.
# >>> x = configure()
# >>> pp(x)
#
# * modify a config dictionary and write it back to the device.
# >>> x['FrequencyStop'] = 2471
# >>> pp(configure(x))
#
def configure(cfg = {}, **argv):
    cfg.update(argv)
    apply(d.setparam, (), cfg)
    return d.getparam()

## Repetitive measurement of distances between a tag and a group of anchors.
#
# @param cnt
#       Number of measurement cycles (default 2).
# @param logfile
#       Name of the logfile (default "atmel_distance.log").
# @param tag
#       Address of the tag (default 2).
# @param anchors
#       List of anchor addresses (default [1,]).
# @param log_style
#       Version of the line log style in the result file.
#       log_style  = 0           : |  n| i1 r1 d1 q1 i1 r2 d2 ....
#       log_style != 0 (default) : |  n| i1 r1 d1 q1 # da1 qa1 da2 qa2 ... # i1 r2 d2 q2 ....
#       Format identifiers:
#           n   = measurement number
#           i1  = initiator address, in this case the address of the mobile tag.
#           rx  = reflector, in this case one of the anchor addresses [x=1...4]
#           dx  = distance between i1 and rx
#           qx  = quality factor for distance dx
#           daN = individual antenna pair or frequency block distance between i1 and rx
#           qaN = individual antenna pair or frequency block quality factor between i1 and rx
#
# @note: If dx = <ERROR CODE> and qx = -1 means anchor or tag are not accessible.
#
# @return The name string of the logfile.
#
# Examples:
#
# * Measure distance, display and log weighted distance and individual results,
#      antenna pairs (1,2, or 4)
# >>> measdist()
# |  0| 1 2 2995 96 # 3004 100 3001 90 2989 100 2999 100
# |  1| 1 2 3005 94 # 3019 90 3004 100 3000 100 3010 90
# closed logfile: atmel_distance.log
# 'atmel_distance.log'
#
# * Measure distance, display and log weighted distance only
# >>> measdist(log_style=0)
#
# * Run N measurements, results logged in logfile="foo.log"
# >>> measdist(cnt=42,logfile="foo.log")
#
# * Measures between tag (2) and multiple anchors, e.g. (1) and (3), if available.
# >>> measdist(anchors=[1,3], tag=2)
#
def measdist(cnt=2, logfile="atmel_distance.log", anchors=[1,], tag=2, log_style=1):
    global d
    try:
        # Disable continuous ranging and suppress debug info
        d.setparam(Verbose = 0)
        d.setparam(FilteringlengthduringcontinuousRanging=1)
        if (log_style == 0):
            d.setparam(ProvideallMeasurementResults=0)
        else:
            d.setparam(ProvideallMeasurementResults=1)
        f = file(logfile,"a")
        for i in range(cnt):
            result = []
            fp     = 1
            for a in anchors:
                d.resultEvent.clear()
                d.ERROR = 0
                run(reflector=tag, initiator=a)
                if d.ERROR == 255:
                    result.append("%s" % 'Timeout: ( 255 )')
                    if (log_style == 0):
                        result.append("%d %d %.0f" % (a, tag, -1))
                    else:
                        result.append("%d %d %.0f %d" % (a, tag, -1, 0))
                else:
                    if d.ERROR != 0:
                        res  = d.result[4]
                        dqf  = -1
                        fail = 0
                    else:
                        res  = d.result[0]
                        dqf  = d.result[1]
                        fail = 1
                #                         anchor-addr   tag-addr     distance     dqf
                res_str = "%d %d %d %d" % (d.result[2], d.result[3], res,         dqf)
                if (log_style == 0):
                    result.append(res_str)
                else:
                    if fp:
                        #                         cnt anchor-addr
                        result.append("%s #" % (res_str))
                    else:
                        result.append("# %s #" % res_str)
                    for dist, dqf in d.results_antenna_div:
                        result.append("%d %d"% (dist*fail, dqf*fail))
                    d.results_antenna_div = []
                    d.no_of_measurements  = 0
                fp = 0
            result = ("|%3d| " % (i)) + " ".join(result) + "\n"
            f.write(result)
            print result,

    except KeyboardInterrupt:
        # Hit Ctrl-C to terminate ongoing measurement.
        pass
    except:
        traceback.print_exc()
    f.close()
    print "closed logfile: %s" % f.name
    return f.name


## Measure the current distance.
#
# @param cnt
#       number of measurement repetitions
# @param initiator
#       16 bit inititator address
# @param reflector
#       16 bit reflector address
#
# Examples:
#
# * Run one remote measurement between an initiator and reflector
# >>> run(initiator=1, reflector=2)
# [{'dqf': 91, 'dist': 2965, 'initiator': 1, 'reflector': 2}]
#
# * Run N = 3 measurements between two nodes
# >>> run(cnt=3, initiator=1, reflector=2)
#
# * use pretty print command to improve result representation
# >>> pp(run(cnt=3, initiator=1, reflector=2))
#
#
def run(cnt=1, **argv):
    global d
    ret = []
    for i in xrange(cnt):
        results = apply(d.run,(),argv)
        ret.append(results)
    return ret

## Display help.
# Usage:
#   help()       ... shows main help message.
#   help(<func>) ... shows the help message for a function.
#
def help(topic = None):
    if topic != None:
        pydoc.help(topic)
    else:
        print __doc__ % VERSION

##
# Calculate Mean and Variance over a array of numbers
def _mean_var_(l):
    tmp = 0.0
    for x in l:
        tmp += x
    mean = tmp / len(l)
    tmp = 0.0
    for x in l:
        tmp += (x-mean)**2
    var = tmp / len(l)
    return (mean, var)

##
# Calculate Median over a array of numbers
def _median_(l):
    l2 = l[:]
    l2.sort()
    median = l2[int(round(len(l2)/2.0))]
    return (median)

##
# Determin Min & Min index of an array of numbers
def _min_(l):
    val = min(l)
    idx = l.index(val)
    return (val, idx)

##
# Determin Max & Max index of an array of numbers
def _max_(l):
    val = max(l)
    idx = l.index(val)
    return (val, idx)

##
# Calculate weighted minimum over a array of numbers
def _min_var_(l):
    M_THRES   = 100
    minimum   = min(l)
    mean, var = _mean_var_(l)
    b         = M_THRES / (M_THRES + var)
    return (b * mean + (1-b) * minimum)

def _get_device_class_():
    try:
        ret = BasicDevice
    except:
        traceback.print_exc()
        ret = BasicDevice
    return ret

## parse the command line options.
def _parse_arguments_():
    global PORT, VERBOSE, INTERNAL
    opts,args = getopt.getopt(sys.argv[1:],"p:hVvI")
    for o,v in opts:
        if o == '-p':
            PORT = v
        if o == '-h':
            help()
            sys.exit(0)
        if o == '-V':
            print "Version %s" % VERSION
            sys.exit(0)
        if o == '-v':
            VERBOSE += 1
        if o == '-I':
            INTERNAL = True
    return args

## Reset the firmware of the board
def reset():
    apply(d.reset, ())
    d.setparam(FilteringlengthduringcontinuousRanging=1)
    return d.getparam()

## Permanent distance measurement and filtering.
#
# @param initiator
#       16 bit inititator address (1)
# @param reflector
#       16 bit reflector address (2)
# @param filter_len
#       filter length (5) : {filter_len >= 2}
# @param filter_mode
#       filter mode  'av': averaging (default)
#                    'me': median search
#                    'mi': minimum search
#                    'ma': maximum search
#                    'mv': weighted minimum search
#
# Stop operation with keyboard interrupt 'Ctrl-C'
#
# Examples:
#
# * Start distance measurement, display final distance averaged over 5 individual distances
# >>> rangefinder()
# Result:
# ---------------------------------------------------------------
#  Dist:   124cm| Spd:  2| Dir: L| DQF:  68%| Dur: 577ms| Err: L|
#
# * Dist:   Distance, weighted [cm]
# * Spd:    Speed indicator
# * Dir:    node moving indication: L-leaving, A-approaching
# * DQF:    DQF, weighted [%]
# * Dur:    Measurement duration including signalling and data transfer from/to Coordinator & host-PC [ms]
# * Err:    Distance validity indication
# *         S   measured distance too short (may caused by moving node)
# *         L   measured distance too long (may caused by moving node)
# *         D   distance ignored because DQF below threshold
# *         T   ranging transaction error
#
# * Start distance measurement, display mean distance derived from 10 individual distances
# >>> rangefinder(filter_len = 10, filter_mode = 'av')
#
def rangefinder(initiator    = 1,       # ranging initiator device
                reflector    = 2,       # ranging reflector device
                filter_len   = 5,       # filtering length / filter history
                filter_mode  = 'av'):   # ranging scheme

    d.setparam(FilteringlengthduringcontinuousRanging=1)
    d.setparam(Verbose = 0)

    # input parameter check
    MIN_FILTER_LEN  = 2
    filter_modes    = ['av', 'me', 'mi', 'ma', 'mv']

    if filter_mode not in filter_modes:
        filter_mode = filter_modes[0]
        print 'NOTE: filter mode set to %s' % filter_mode

    if (filter_len < MIN_FILTER_LEN):
        filter_len = MIN_FILTER_LEN
        print 'NOTE: filter_len set to %d' % filter_len

    # set default parameters
    speed_max           = 200.0     # max speed [cm/sec] 200 -> 7.2 km/h
    ranging_array_idx   = 0         # distance/dqf array index
    fill_status         = 0         # array fill status
    time_diff_dist      = 0         # time between two distance measurements

    temp_filter_len     = filter_len        # temporary array length
    Dist_array          = filter_len*[-1]   # dist history
    Dqf_array           = filter_len*[-1]   # dqf history
    time_history_idx    = 0         # time history array index
    Time_history        = 2*[0.0]   # time history
    Dist_history        = 2*[-1]    # distance result history -> calc speed
    speed_array_len     = 4         # length of speed history array
    Speed_array         = speed_array_len*[0]   # speed history array -> average speed_array_len speed values

    print "Result:\n" + 63*"-"
    while (1):
        try:
            # get distance and time stamp
            Time_history[time_history_idx] = time.time()
            res = run(initiator=initiator, reflector=reflector)[0]

            time.sleep(0.1) # limit ranging cycle time
            if (fill_status == 0): # instant fill
                Dist_array = len(Dist_array)*[res['dist']]
                Dqf_array = len(Dqf_array)*[res['dqf']]
                Time_history = len(Time_history)*[Time_history[time_history_idx]]
                Dist_filt = Dist_array[0]  # filtered distance result
                Dqf_filt = Dqf_array[0]  # filtered DQF result
                fill_status = 1     # array fill indicator
                err = ""
            else:
                # time for a measurement cycle and check
                time_diff_dist = Time_history[time_history_idx]-Time_history[(time_history_idx-1)%len(Time_history)]
                Dist_array[ranging_array_idx], err  = _check_dist_(res['dist'], res['dqf'], Dist_filt, speed_max*time_diff_dist)
                Dqf_array[ranging_array_idx]        = res['dqf']
                # preparation for short term speed estimation
                Dist_history[0], t    = _mean_var_([Dist_array[(ranging_array_idx-0)%len(Dist_array)], Dist_array[(ranging_array_idx-1)%len(Dist_array)]])
                Dist_history[1], t    = _mean_var_([Dist_array[(ranging_array_idx-2)%len(Dist_array)], Dist_array[(ranging_array_idx-3)%len(Dist_array)]])
                ranging_array_idx += 1
                if(ranging_array_idx  == temp_filter_len) : ranging_array_idx  = 0
                time_history_idx += 1
                if(time_history_idx  == len(Time_history)) : time_history_idx  = 0

            if (filter_mode == 'av'):            # average of dist and dqf
                Dist_filt, t = _mean_var_(Dist_array)
                Dqf_filt, t = _mean_var_(Dqf_array)
            elif (filter_mode == 'me'):          # median of dist and dqf
                Dist_filt    = _median_(Dist_array)
                Dqf_filt    = _median_(Dqf_array)
            elif (filter_mode == 'mi'):          # minimum of dist and dqf
                Dist_filt, t = _min_(Dist_array)
                Dqf_filt    = Dqf_array[t]
            elif (filter_mode == 'ma'):          # maximum of dist and dqf
                Dist_filt, t = _max_(Dist_array)
                Dqf_filt    = Dqf_array[t]
            elif  (filter_mode == 'mv'):         # minimum of dist and dqf considering variance
                Dist_filt    = _min_var_(Dist_array)
                Dqf_filt    = _min_var_(Dqf_array)
            else:
                print "Strategy not supported"
                return
            if (time_diff_dist != 0):
                Speed_array[ranging_array_idx%speed_array_len] = (Dist_history[0]-Dist_history[1])/time_diff_dist/100*3.6   # cm/s -> km/h (estimate)
                speed_filt, t   = _mean_var_(Speed_array)
                speed_filt      = int(round(speed_filt))  # int
            else:
                speed_filt = 0
            if (speed_filt < -1):
                mov_dir = 'A'           # node approaches
            elif (speed_filt > 1):
                mov_dir = 'L'           # node leaves
            else:
                mov_dir = ''            # constant node position
            res_str = "Dist: %5dcm| Spd: %2s| Dir: %1s| DQF: %3d%%| Dur: %3dms| Err: %1s| " % (Dist_filt, speed_filt, mov_dir, Dqf_filt, time_diff_dist*1000, err)
            print (res_str + (len(res_str)+1)*"\b"),
            if VERBOSE:
                print "S:%d, S:[%1.2f %1.2f]" % (speed_filt, Speed_array[0], Speed_array[1]), Dist_filt, Dqf_filt, Dist_array
        except (KeyboardInterrupt, SystemExit):
            # Hit Ctrl-C to terminate ongoing measurement.
            raise

## check distance validity
def _check_dist_(Dist_array, Dqf_array, Dist_filt, Dist_limit):
    Q_THRESHOLD = 10                                # DQF threshold to ignore distance with low DQF
    if (Dist_array == -1):                          # Ranging transaction error
        return (Dist_filt, "T")
    elif (Dqf_array < Q_THRESHOLD):                 # Ignore distance with low DQF
        return (Dist_filt, "D")
    elif ((Dist_filt - Dist_array)>(Dist_limit)):   # Equalize too short distances
        return ((Dist_filt - Dist_limit), "S")
    elif ((Dist_array - Dist_filt)>(Dist_limit)):   # Equalize too long distances
        return ((Dist_filt + Dist_limit), "L")
    else:
        return (Dist_array, "")

# === python shell setup ======================================================
try:
    import readline, atexit
    HISTORY = "ranging.hist"
    readline.parse_and_bind("tab:complete")
    atexit.register(readline.write_history_file, HISTORY)
    try:
        readline.read_history_file(HISTORY)
    except:
        pass
except:
    pass

if sys.platform.find("linux") < 0:
    atexit.register(raw_input, "Type Enter to Exit")

pp = pprint.pprint
ppd = lambda x : pp(dir(x))

# === main function ===========================================================
if __name__ == "__main__":
    args = _parse_arguments_()

    # ask for a serial port, if not specified with -p.
    if PORT == None:
        PORT = raw_input("Enter port name:")

    Device = _get_device_class_()
    d = Device(PORT)
    configure(Verbose = VERBOSE)

    # execute user scripts
    for a in args:
        execfile(a)

# === EOF =====================================================================
