package com.visybl.cloudnode;



import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import com.pi4j.io.serial.Serial;
import com.pi4j.io.serial.SerialDataEvent;
import com.pi4j.io.serial.SerialDataEventListener;
import com.pi4j.io.serial.SerialFactory;
import com.pi4j.io.serial.SerialPortException;
import com.visybl.api.Beacon;

public abstract class NfcScanner
{
	static Logger log = Logger.getLogger(NfcScanner.class);
	static Serial serial;
	
	static byte[] ack = { (byte) 0x00, (byte) 0x00, (byte) 0xff, (byte) 0x00, (byte) 0xff, (byte) 0x00 };
	
	static byte[] timeout = { (byte) 0x00, (byte) 0x00, (byte) 0xff, (byte) 0x03, (byte) 0xfd, (byte) 0xd5, (byte) 0x61, (byte) 0x00, (byte) 0xca, (byte) 0x00 };
	
	static byte[] failRead = {
			(byte) 0x00, 
			(byte) 0x00, 
			(byte) 0xff,  
			(byte) 0x00, 
			(byte) 0xff,
			(byte) 0x00, 
			(byte) 0x00, 
			(byte) 0x00, 
			(byte) 0xff,
			(byte) 0x03, 
			(byte) 0xfd, 
			(byte) 0xd5, 
			(byte) 0x41, 
			(byte) 0x01, 
			(byte) 0xe9, 
			(byte) 0x00 };
	
	static byte[] preamble_StartOfPkt = { (byte) 0x00, (byte) 0x00, (byte) 0xff};
	
	static byte[] wakeUP = { 
			(byte) 0x55,
			(byte) 0x55,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x00 };
	
	static byte[] SAMConfiguration = { 
			(byte) 0x00, 
			(byte) 0x00, 
			(byte) 0xff, 
			(byte) 0x03, 
			(byte) 0xfd, 
			(byte) 0xd4,
			(byte) 0x14, 
			(byte) 0x01, 
			(byte) 0x17, 
			(byte) 0x00 };
	
	static byte[] diagnose = {
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0xff,
			(byte) 0x09,
			(byte) 0xf7,
			(byte) 0xd4,
			(byte) 0x00,
			(byte) 0x00,
			(byte) 0x6c,
			(byte) 0x69,
			(byte) 0x62,
			(byte) 0x6e,
			(byte) 0x66,
			(byte) 0x63,
			(byte) 0xbe,
			(byte) 0x00
					
					
	};
	
	static byte[] getFW_cmd = { (byte) 0x00, (byte) 0x00, (byte) 0xff, (byte) 0x02, (byte) 0xfe, (byte) 0xd4,
			(byte) 0x02, (byte) 0x2a, (byte) 0x00 };
	
	static byte[] InListPassiveTarget_cmd = { (byte) 0x00, (byte) 0x00, (byte) 0xff, (byte) 0x04, (byte) 0xfc, (byte) 0xd4,
			(byte) 0x4a, (byte) 0x01, (byte) 0x00, (byte) 0xe1, (byte) 0x00 };
	
	
	
	static byte[] InAutoPoll_cmd = { 
			(byte) 0x00, 
			(byte) 0x00, 
			(byte) 0xff, 
			(byte) 0x0a,
			(byte) 0xf6, 
			(byte) 0xd4, 	
			(byte) 0x60, 
			(byte) 0x14,
			(byte) 0x02,			
			(byte) 0x20, 
			(byte) 0x10, 
			(byte) 0x03,			
			(byte) 0x11, 
			(byte) 0x12,
			(byte) 0x04,			
			(byte) 0x5c,		
			(byte) 0x00 }; 
	
	static byte[] InDataExchange_cmd = { (byte) 0x00, (byte) 0x00, (byte) 0xff, (byte) 0x05, (byte) 0xfb, (byte) 0xd4,
			(byte) 0x40, (byte) 0x01, (byte) 0x30,  (byte) 0x12, (byte) 0xa9,(byte) 0x00 };
	
	static long nfcPollTimestamp=0;
    
	static void startNfc()
	{
		log.info(Thread.currentThread().getId()+"- NfcScanner(): In startNfc()");
		try
		{
			if(serial!=null)
			{
				serial.removeListener(pn532SerialDataListener);
				if(serial.isOpen())
				{
					serial.flush();
					serial.close();
				}
			
			}
			
			Thread.sleep(500);
			
			serial = SerialFactory.createInstance();
			
			long now = System.currentTimeMillis();
			while((System.currentTimeMillis() - now) < 5000)
			{
				serial.open("/dev/ttyS0", 115200);
				if(serial.isOpen())
					break;
				else
					Thread.sleep(1000);
			}
			
			log.info("NfcScanner.startNfc(): Port Opened: " + serial.isOpen());
						
			//03/09 - Copied from Luderic code
			
			serial.write(wakeUP);
			serial.flush();
			
			serial.write(SAMConfiguration);
			serial.flush();
			
			serial.write(diagnose);
			serial.flush();
			
			serial.write(InAutoPoll_cmd);
			serial.flush();
			
			serial.addListener(pn532SerialDataListener);
						
			log.info("NfcScanner.startNfc(): Complete ");
		}
		catch (SerialPortException e)
		{
			log.error("NfcScanner.startNfc(): Serial port Exception in constructor: " + e.toString());
			
		}
		catch (Exception ex)
		{
			log.error("NfcScanner.startNfc(): Exception in constructor: " + ex.toString());
			
		}
	}
	
	
	static volatile boolean readyForNextCmd = false, receivedAck = false;

	// Getting stuck in this 
/*
[TRACE] 07-10-2020 12:41:28.539 - NfcScanner.dataReceived(): Recd: 0000ff03fdd56100ca00
[TRACE] 07-10-2020 12:41:28.539 - NfcScanner.dataReceived(): autopoll timeout
[TRACE] 07-10-2020 12:41:28.592 - NfcScanner.dataReceived(): Recd: 0000ff00ff00
[TRACE] 07-10-2020 12:41:28.592 - NfcScanner.dataReceived(): received ack=true
[TRACE] 07-10-2020 12:42:04.578 - NfcScanner.dataReceived(): Recd: 0000ff03fdd56100ca00
[TRACE] 07-10-2020 12:42:04.578 - NfcScanner.dataReceived(): autopoll timeout
[TRACE] 07-10-2020 12:42:04.631 - NfcScanner.dataReceived(): Recd: 0000ff00ff00
[TRACE] 07-10-2020 12:42:04.631 - NfcScanner.dataReceived(): received ack=true
 */
	protected static SerialDataEventListener pn532SerialDataListener = new SerialDataEventListener()
	{
		@Override
		public void dataReceived(SerialDataEvent eventData)
		{
			
			nfcPollTimestamp = System.currentTimeMillis();

			try
			{	
				
				String data = eventData.getString(Charset.forName("ISO-8859-1"));
				
				final byte[] array;
				String ackFrame = Helpers.bytesToHexString(ack);
				
				array = data.getBytes("ISO-8859-1");
				
				log.trace("NfcScanner.dataReceived(): Recd: " + Helpers.bytesToHexString(array));
				
				
				if(Helpers.bytesToHexString(array).compareTo(ackFrame)==0)
				{
					log.trace("NfcScanner.dataReceived(): received ack=true");
					
					receivedAck = true;
				}
				else if (Helpers.bytesToHexString(array).compareTo(Helpers.bytesToHexString(timeout))==0)
				{

					log.trace("NfcScanner.dataReceived(): autopoll timeout");
					
					serial.write(InAutoPoll_cmd);
					
				}
				else if (Helpers.bytesToHexString(array).compareTo(Helpers.bytesToHexString(failRead))==0)
				{

					log.trace("NfcScanner.dataReceived(): failRead ... Starting Autopoll again.");
					
					serial.write(InAutoPoll_cmd);
					
				}
				else if(Helpers.bytesToHexString(array).length() > ackFrame.length())						
				{
					log.trace("NfcScanner.dataReceived(): received response");
					

					
					try
					{
						processResponse(array);
					}
					catch(Exception ex)
					{					
						log.error("NfcScanner.dataReceived() Exception: " + ex.toString());
						cleanup();
					}
					
					readyForNextCmd = true;
				}

				
			}
			catch (UnsupportedEncodingException e)
			{
				// error in receiving data
				log.error("NfcScanner.dataReceived(): " + e.toString());
				cleanup();
			}
			catch (IOException e)
			{
				// error in receiving data
				log.error("NfcScanner.dataReceived(): " + e.toString());
				cleanup();
			}
		}
	};
	
	static void processResponse(byte[] response)
	{
		String responseString = Helpers.bytesToHexString(response);
		
		String prefixFrame = Helpers.bytesToHexString(preamble_StartOfPkt);
		
		int lastIndexOfPrefixFrame = responseString.lastIndexOf(prefixFrame);
		
		if(lastIndexOfPrefixFrame!=-1)
		{
			// remove ACK prefix
			responseString = responseString.substring(lastIndexOfPrefixFrame + prefixFrame.length());
		}
		
		// look for error packets
		
		log.info("NfcScanner.processResponse(): responseString = " + responseString);
		
		if(responseString.contains("d50332"))
		{
			log.info("NfcScanner.processResponse(): getFW ping OK");
			//lastPingResponseTime = System.currentTimeMillis();
		}
		else if(responseString.contains("d56102"))
		{
			log.info("NfcScanner.processResponse(): > 1 NFC tags");
			//lastAutoPollResponseTime = System.currentTimeMillis();
		}
		else if(responseString.contains("d56101"))
		{	
			//lastAutoPollResponseTime = System.currentTimeMillis();
			
			//11 EF D5 61 01 [10] 0C 01 00 44 00 07 04 37 4B D2 9C 39 80 B4 00
			int loc = 5;
			int len = 1;
		
			log.trace("NfcScanner.processResponse(): finding targetType");
			String targetType = responseString.substring(loc*2, (loc+len-1)*2+2);
			
			//11 EF D5 61 01 10 0C 01 [00 44] 00 07 04 37 4B D2 9C 39 80 B4 00
			loc += 3;
			len = 2;
			log.trace("NfcScanner.processResponse(): finding SENS_RES");
			String SENS_RES = responseString.substring(loc*2, (loc+len-1)*2+2);
			
			//11 EF D5 61 01 10 0C 01 00 44 [00] 07 04 37 4B D2 9C 39 80 B4 00
			loc += 2;
			len = 1;
			log.trace("NfcScanner.processResponse(): finding SEL_RES");
			String SEL_RES = responseString.substring(loc*2, (loc+len-1)*2+2);
			
			//11 EF D5 61 01 10 0C 01 00 44 00 [07] 04 37 4B D2 9C 39 80 B4 00
			loc += 1;
			len = 1;
			String Length = null, NFCID = null;
			log.trace("NfcScanner.processResponse(): finding Length");
			Length = responseString.substring(loc*2, (loc+len-1)*2+2);
		
			//11 EF D5 61 01 10 0C 01 00 44 00 07 [04 37 4B D2 9C 39 80] B4 00
			loc += 1;
			len = Integer.parseInt(Length, 16);
			log.trace("NfcScanner.processResponse(): finding NFCID");
			NFCID = responseString.substring(loc*2, (loc+len-1)*2+2);
			
			startTime = System.currentTimeMillis();
			log.info(startTime + ": NfcScanner.processResponse(): Type: " + targetType + ", ATQA (SENS_RES): "
					+ SENS_RES + ", SAK (SEL_RES): " + SEL_RES + ", UID (1): " + NFCID);
			
			readingBlocks = true;
			currentBlock = 0;
			readBlock(currentBlock);
			
		}
		
																 
		else if(responseString.contains("edd54100")) // block reads
		{
			log.info("NfcScanner.processResponse(): Read blocks " + currentBlock + "-" + (currentBlock+3));
			
			int ndefMarkerByteLoc = 17;
			
			if(currentBlock==0 && responseString.substring(ndefMarkerByteLoc*2, ndefMarkerByteLoc*2+2).compareTo("e1")==0)
			{
				
				int ndefMappingVersionByteLoc = 18;
				int tagMemSizeByteLoc = 19;
				int tagAccessRestrictionsByteLoc = 20;
				log.info("NfcScanner.processResponse(): Contains NDEF, "
						+ "mapping version " + responseString.substring(ndefMappingVersionByteLoc*2, ndefMappingVersionByteLoc*2+2)
						+ ", tag mem size = " + responseString.substring(tagMemSizeByteLoc*2, tagMemSizeByteLoc*2+2)
						+ ", access restrictions = " + responseString.substring(tagAccessRestrictionsByteLoc*2, tagAccessRestrictionsByteLoc*2+2));
				
				// read the next 4 blocks
				readBlock(currentBlock += blockSize);
			}
			else if(currentBlock > 3 && currentBlock < 8 /*&& responseString.contains("edd54100")*/)
			{	
				int ndefStartOfData = 0;
				
				int firstTlvLoc = 5; // first TLV assumed to start at beginning of block 4
				int index = firstTlvLoc; 
				
				// Looking for NDEF start in the while loop
				while(ndefStartOfData==0 && index < responseString.length()/2 && index < 100)
				{
					String byteStr = responseString.substring(index*2, index*2+2);
					log.trace("NfcScanner.processResponse(): TLV tag found = " + byteStr);
					
					// TLV definitions: Drive/Engineering/NFC/NFCForum-TS-Type-2-Tag_1.1.pdf, Section 2.3, Table 2
					if(byteStr.compareTo("00")==0) // NULL TLV
					{
						continue;
					}
					else if(byteStr.compareTo("01")==0 || byteStr.compareTo("02")==0 || byteStr.compareTo("fd")==0)
					{
						log.trace("NfcScanner.processResponse(): Ignorable TLV found");
						index++;
						int tlvLenByte = Integer.parseInt(responseString.substring(index*2, index*2+2),16);
						index+=tlvLenByte;
					}
					else if(byteStr.compareTo("03")==0) // start of NDEF
					{
						log.info("NfcScanner.processResponse(): Found NDEF start at " + index);
						ndefStartOfData = index;
					}
					
					index++;
				}

				if(ndefStartOfData > 0)  // NDEF start found
				{	
					int ndefLenByteLoc = ndefStartOfData + 1;
					int ndefWellKnownTypeRecordByteLoc = ndefStartOfData + 5;
					int ndefStatusByteLoc = ndefWellKnownTypeRecordByteLoc + 1; // Drive/Engineering/NFC/NFCForum-TS-NDEF.pdf, Section 3.2.1, Table 3
					
					int ndefStatusByte = Integer.parseInt(responseString.substring(ndefStatusByteLoc*2, ndefStatusByteLoc*2+2),16);
					// bits 0-5 are length of IANA language code
					int langCodeLen = ndefStatusByte & 0x3f;
					log.trace("NfcScanner.processResponse(): langCodeLen=" + langCodeLen);
					ndefStartOfData = ndefStatusByteLoc + langCodeLen + 1; // actual start of NDEF payload
					
					// skipping the prefix "d1 01 07 54 00 ..."
					ndefDataLen = Integer.parseInt(responseString.substring(ndefLenByteLoc*2, ndefLenByteLoc*2+2),16) - (5 + langCodeLen); 
					ndefDataBytesFound = 0;
					
					ndefData = new StringBuilder();
					
					int ndefWellKnownTypeRecord = Integer.parseInt(responseString.substring(ndefWellKnownTypeRecordByteLoc*2, ndefWellKnownTypeRecordByteLoc*2+2),16);
					log.info("NfcScanner.processResponse(): Found NDEF, length = " + ndefDataLen
							+ ", record type = " + (char)ndefWellKnownTypeRecord);
					
					int endIndex = ndefStartOfData + (ndefDataLen - ndefDataBytesFound) - 1;
					
					log.info("NfcScanner.processResponse(): endIndex = " + endIndex);
					
					if(endIndex > (responseString.length()/2 - 1 - 2)) // Leaving out last 2 bytes, NDEF payload continues to next set of blocks 
					{
						log.info("NfcScanner.processResponse(): NDEF continues to next set of blocks (current block = " + currentBlock + ")");
						ndefData.append(responseString.substring(ndefStartOfData*2, (responseString.length()/2-3)*2+2));
						ndefDataBytesFound = ndefData.length()/2;
						log.info("NfcScanner.processResponse(): NDEF data = " + ndefData + " [" + ndefDataBytesFound + "/"+ ndefDataLen + "]" );
						readBlock(currentBlock += blockSize);
						
					}
					else // All NDEF payload is contained in this read
					{
						boolean ndefEndMarkerFound = (responseString.substring((endIndex+1)*2, (endIndex+1)*2 + 2).compareToIgnoreCase("fe")==0);
						
						log.info("NfcScanner.processResponse(): NDEF contained in this set of blocks (current block = "
								+ currentBlock + "), FE end marker found = " + ndefEndMarkerFound);
						
						ndefData.append(responseString.substring(ndefStartOfData*2, (endIndex+1)*2));
						
						log.info("NfcScanner.processResponse(): NDEF = " + ndefData.toString());
						
						onEndOfNdefFound();
					}
				}
				else // NDEF start not found, abort
				{
					log.info("NfcScanner.processResponse(): NDEF start not found, aborting");
					cleanup();
				}
			}
			// Process blocks 8-11, 12-15 etc
			else
			{
				int ndefStartOfData = 5; // Skipping prefix "13 ED D5 41 00 ... "
				
				int endIndex = ndefStartOfData + (ndefDataLen - ndefDataBytesFound) - 1;
				log.info("NfcScanner.processResponse(): endIndex = " + endIndex);
				
				if(endIndex > (responseString.length()/2 - 1 - 2)) // Leaving out last 2 bytes, NDEF payload continues to next set of blocks 
				{
					log.info("NfcScanner.processResponse(): NDEF continues to next set of blocks (current block = " + currentBlock + ")");
					ndefData.append(responseString.substring(ndefStartOfData*2, (responseString.length()/2-3)*2+2));
					ndefDataBytesFound = ndefData.length()/2;
					log.info("NfcScanner.processResponse(): NDEF data = " + ndefData + " [" + ndefDataBytesFound + "/"+ ndefDataLen + "]" );
					readBlock(currentBlock += blockSize);
					
				}
				else // All NDEF payload is contained in this read
				{
					
					boolean ndefEndMarkerFound = (responseString.substring((endIndex+1)*2, (endIndex+1)*2 + 2).compareToIgnoreCase("fe")==0);
					
					log.info("NfcScanner.processResponse(): NDEF contained in this set of blocks (current block = "
							+ currentBlock + "), FE end marker found = " + ndefEndMarkerFound);
					
					ndefData.append(responseString.substring(ndefStartOfData*2, (endIndex+1)*2));
					
					log.info("NfcScanner.processResponse(): NDEF = " + ndefData.toString());
					
					onEndOfNdefFound();
				}
			}
		}

	}
	
	static volatile boolean readingBlocks = false;
	static final int blockSize = 4;

	static int ndefDataLen, ndefDataBytesFound;
	static volatile int currentBlock = -1;
	static StringBuilder ndefData = null;
	
	//static Timer abortBlockReadTimer = null;
	
	static long startTime;
	
	static void readBlock(int block)
	{
		InDataExchange_cmd[9] = (byte)block;
		
		byte sumOfBytes = 0x00;
		for(int i=0; i < 5; i++)
		{
			sumOfBytes += InDataExchange_cmd[5 + i];
		}
		byte sumOfBytesTwosComplement = (byte) Helpers.twosComplement(sumOfBytes);
		
		InDataExchange_cmd[10] = sumOfBytesTwosComplement;
		
		
		StringBuilder sb = new StringBuilder("InDataExchange blk " + String.format("%02x", InDataExchange_cmd[9]) + "\t");
		for (int i = 0; i < InDataExchange_cmd.length; i++)
		{
			sb.append(String.format("%02X ", InDataExchange_cmd[i]));
		}
		log.info("NfcScanner.readBlock(): " + sb.toString());
		
		try
		{
			serial.write(InDataExchange_cmd);
		}
		catch (IllegalStateException e)
		{
			log.error("NfcScanner.readBlock(): " + e.toString());
		}
		catch (IOException e)
		{
			log.error("NfcScanner.readBlock(): " + e.toString());
		}	
	}
	
	static int count = 1;
	static private void onEndOfNdefFound()
	{
		final String tagId = new String((Helpers.hexStringToBytes(ndefData.toString())));
		
		long now = System.currentTimeMillis();
		long duration =  now - startTime;
		
		log.info(count++ + ": NfcScanner.onEndOfNdefFound(): Final NDEF = [" + tagId.length()/2 + " bytes] "
		 + ", duration " + duration + ", " + tagId);
		
		
		// Check if tagId is listed in local file
		if(isRegistered(tagId))
		{
	    	// Prevent the buzzer going off multiple times
	    	if(lastNfcId.compareToIgnoreCase(tagId)!=0|| (System.currentTimeMillis() - lastNfcPostTime) > 10000)
	    	{
	    		lastNfcId = tagId;
		    	// Record the time
		    	lastNfcPostTime = System.currentTimeMillis();
		    	
				new Thread(new Runnable()
			    {
			        @Override
			        public void run()
			        {	
			        	boolean state = false;
			        	if(Constants.nfcEntryExit)
			        	{
				        	// toggle state, get current state
				        	state = toggleNfcState(tagId);
				        	if(state)
				        	{
				        		log.info("NfcScanner.onEndOfNdefFound().run():" + tagId + ": Inside");
				        	}
				        	else
				        	{
				        		log.info("NfcScanner.onEndOfNdefFound().run():" + tagId + ": Outside");
				        	}
			        	}
			        	
			        	// do the buzz
			        	log.info("postNfcRead, buzzing");
		         		// 2 beeps for outside, 1 beep for inside
		         		int count = state ? 1 : 2;
		         		nfcBuzzAndBlink(count);
		         		
			            postNfcRead(tagId);
			        }
			    }).start();
	    	}
	    	else
	    	{
	    		log.info("NfcScanner.onEndOfNdefFound(): skipping, time since previous = " + (System.currentTimeMillis() - lastNfcPostTime));
	    		


	    	}
		}
		else
		{
			log.info("NfcScanner.onEndOfNdefFound(): NFC ID " + tagId + " not registered, skipping");
			
    		//03/05/2020 - Arun added this code to turn LED red if nfc is not authorized
			
			
    		
    		LedThread.gpioPinDigitalOutputs[LedThread.LED_GREEN].low();
			LedThread.gpioPinDigitalOutputs[LedThread.LED_RED].high();
			
			try
			{
				Thread.sleep(2000);
			}
			catch (InterruptedException e)
			{
				
				//ignore
			}
		
			LedThread.gpioPinDigitalOutputs[LedThread.LED_GREEN].high();  // turns LED back on
			LedThread.gpioPinDigitalOutputs[LedThread.LED_RED].low();
			
			postNfcRead(tagId);
		}
		
		cleanup();
	}
	
	static private void cleanup()
	{
		log.info("In NfcScanner.cleanup()");
		
		// cleanup, reset etc after block read is done
		ndefData = null;
		readingBlocks = false;
		
		if(serial!=null)
		{
			try
			{
				log.info("NfcScanner.cleanup(): serial is not NULL, serial.isOpen()=" + serial.isOpen());
				serial.flush();
				
				log.info("NfcScanner.cleanup(): starting InAutoPoll_cmd");
				serial.write(InAutoPoll_cmd);
			}
			catch (IllegalStateException e)
			{
				log.debug("NfcScanner.cleanup(): " + e.toString());
			}
			catch (IOException e)
			{
				log.debug("NfcScanner.cleanup(): " + e.toString());
			}
			
		}
		else
		{
			log.debug("NfcScanner.cleanup(): serial is NULL");
		}
		
		
		startTime = 0;
	}

	
	static long lastNfcPostTime = 0;
	static String lastNfcId = "";
	static void postNfcRead(String tagId)
	{
		String prefix = "0201061AFF4C000215A8D278E435B544EF9CAC0F3FC511FE3E01";
		String suffix = "C5";
		
		// turn this into a mock BLE Adv
		byte []data = Helpers.hexStringToBytes(prefix + tagId + suffix);

		log.info("postNfcRead, data = " + Helpers.bytesToHexString(data));
		
		Beacon nfcBcn = Beacon.parseFromBytes("FC:FC:FC:FC:FC:FC", -40, data);
		
		iBeacon iB = new iBeacon(nfcBcn.getBytes());
		
		
		String virtualName = String.format("%04X",iB.getMajor()).substring(2, 4) + String.format("%04X",iB.getMinor());
        nfcBcn.setDeviceName(virtualName);
        nfcBcn.setReadPointAddress("nfc");
		
    	String jsonPostStr = CloudApp.getBeaconAsJsonObject(nfcBcn, Constants.EVENT_VISIBLE).toString();    	 
    	
    	int postStatus = Helpers.doPost(jsonPostStr, !Constants.offlineMode && Constants.retryFailedPosts);
        boolean updateSuccess = (postStatus > 0);
        if (!updateSuccess)
        {
        	log.info("Failure posting " + nfcBcn);
        }
        else
        {
        	
        }

	}
	
	static boolean toggleNfcState(String nfcTagId)
	{
		try
		{
			File inFile = new File(Constants.HOME_DIRECTORY, "in/"+nfcTagId);
			File outFile = new File(Constants.HOME_DIRECTORY, "out/"+nfcTagId);
			
			if(inFile.exists())
			{
				FileUtils.touch(outFile);
				inFile.delete();
				return false;
			}
			else
			{
				FileUtils.touch(inFile);
				outFile.delete();
				return true;
			}
		}
		catch (IOException e)
		{
			log.info("NfcScanner.toggleNfcState(): " + e.toString()); 
			return true;
		}
	}
	
	/*
	 * Valid NFC IDs are stored locally
	 * to speed up response times
	 * @return true if tag Id  exists in file (case-sensitive)
	 */
	static boolean isRegistered(String nfcTagId)
	{
		if(!Constants.nfcCheckId) return true;

  		return(Constants.nfc_include.contains(nfcTagId.toUpperCase()));
	}
	
	static void nfcBuzzAndBlink(final int count)
	{	 
		new Thread(new Runnable()
		{
			
			@Override
			public void run()
			{
				boolean ledUseOk = false;
				
				if(NodeApp.ledThread==null || !NodeApp.ledThread.isAlive())
				{
		        	if(!LedThread.inUse)
		        	{
		        		LedThread.inUse = true;
		        		ledUseOk = true;
		        	}
				}
				
				// Turn on turnstile relay
				long turnstileRelayHighAtMillis = System.currentTimeMillis();
				LedThread.turnstileGpioPinDigitalOutput.high();
				log.info("NfcScanner.nfcBuzzAndBlink(): turnstile relay ON at " + turnstileRelayHighAtMillis); 
				
				int _count = count;
				while(_count > 0)
				{
					if(LedThread.buzzerGpioPinDigitalOutput!=null)
			 		{
			 			LedThread.buzzerGpioPinDigitalOutput.low(); // turns on buzzer
			 			
			 			if(ledUseOk && LedThread.gpioPinDigitalOutputs[LedThread.LED_GREEN]!=null)
		        		{
			 				log.info("NfcScanner.nfcBuzzAndBlink(): green off, count  = " + count); 
		        			LedThread.gpioPinDigitalOutputs[LedThread.LED_GREEN].low(); // turns off LED
		        		}
			 			
			 			try
						{
								Thread.sleep(Constants.nfcBuzzerTimeMillis);
						}
						catch (InterruptedException e)
						{
							log.debug("Error at nfcBuzzAndBlink: LedThread: " + e.toString());
						}
			 			
			 			LedThread.buzzerGpioPinDigitalOutput.high(); // turns off buzzer
			 			
			 			if(ledUseOk && LedThread.gpioPinDigitalOutputs[LedThread.LED_GREEN]!=null)
		        		{	
			 				log.info("NfcScanner.NfcScanner.nfcBuzzAndBlink(): green on, count  = " + count); 
		        			LedThread.gpioPinDigitalOutputs[LedThread.LED_GREEN].high();  // turns LED back on
		        		}
			 		}
					
					_count--;
					
					try
					{
							Thread.sleep(Constants.nfcBuzzerPauseMillis);
					}
					catch (InterruptedException e)
					{
							
						log.debug("Error at nfcBuzzAndBlink: LedThread: " + e.toString());
					}
					
					// if 1 sec or more since turnstile relay has been
					// turned on, drop it back
					if(turnstileRelayHighAtMillis > 0 && (System.currentTimeMillis() - turnstileRelayHighAtMillis) > 1000)
					{
						LedThread.turnstileGpioPinDigitalOutput.low();
						log.info("NfcScanner.nfcBuzzAndBlink(): turnstile relay OFF");
						turnstileRelayHighAtMillis = 0; //used as a check to see if relay is already off
					}
				}
				
				// if the above loop did not take 1 sec, relay may be still high
				
				if(turnstileRelayHighAtMillis > 0)
				{
					// Wait for remaining time
					long relayRemainingTimeMillis = 1000 - (System.currentTimeMillis() - turnstileRelayHighAtMillis);
					if(relayRemainingTimeMillis > 0)
					{
						try
						{
								Thread.sleep(relayRemainingTimeMillis);
						}
						catch (InterruptedException e)
						{
								
								log.debug("Error at nfcBuzzAndBlink: turnstileRelayHighAtMillis: " + e.toString());
						}
					}
					
					LedThread.turnstileGpioPinDigitalOutput.low();
					log.info("NfcScanner.nfcBuzzAndBlink(): turnstile relay OFF");
					turnstileRelayHighAtMillis = 0; // used as a check to see if relay is already off
				}
				else
				{
					log.info("NfcScanner.nfcBuzzAndBlink(): turnstile relay already OFF");
				}
				
				// release the LED
				if(ledUseOk)
				{
					LedThread.inUse = false;
				}
			}
		}).start();
	}
}
	

	

