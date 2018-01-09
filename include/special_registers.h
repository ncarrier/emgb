/*
 * Original documentation: GBCPUman.pdf, section 2.13.1
 */

#ifndef INCLUDE_SPECIAL_REGISTERS_H_
#define INCLUDE_SPECIAL_REGISTERS_H_

enum special_register {
	SPECIAL_REGISTER_INVALID = 0u,
	SPECIAL_REGISTER_FIRST = 0xff00u,

	/*
	 * Register for reading joy pad info and determining system type (R/W)
	 *
	 * Bit 7 - Not used
	 * Bit 6 - Not used
	 * Bit 5 - P15 out port
	 * Bit 4 - P14 out port
	 * Bit 3 - P13 in port
	 * Bit 2 - P12 in port
	 * Bit 1 - P11 in port
	 * Bit 0 - P10 in port
	 *
	 * This is the matrix layout for register $FF00:
	 *
	 *          P14        P15
	 *           |          |
	 * P10-------O-Right----O-A
	 *           |          |
	 * P11-------O-Left-----O-B
	 *           |          |
	 * P12-------O-Up-------O-Select
	 *           |          |
	 * P13-------O-Down-----O-Start
	 *           |          |
	 */
	SPECIAL_REGISTER_P1 = SPECIAL_REGISTER_FIRST,
	/*
	 * Serial transfer data (R/W)
	 *
	 * 8 Bits of data to be read/written
	 */
	SPECIAL_REGISTER_SB = 0xff01u,
	/*
	 * SIO control (R/W)
	 *
	 * Bit 7 - Transfer Start Flag
	 *         0: Non transfer
	 *         1: Start transfer
	 * Bit 0 - Shift Clock
	 *         0: External Clock (500KHz Max.)
	 *         1: Internal Clock (8192Hz)
	 *
	 * Transfer is initiated by setting the Transfer Start Flag.
	 * This bit may be read and is automatically set to 0 at the end of
	 * Transfer.
	 *
	 * Transmitting and receiving serial data is done simultaneously.
	 * The received data is automatically stored in SB.
	 */
	SPECIAL_REGISTER_SC = 0xff02u,
	/*
	 * Divider Register (R/W)
	 *
	 * This register is incremented 16384 (~16779 on SGB) times a second.
	 * Writing any value sets it to $00.
	 */
	SPECIAL_REGISTER_DIV = 0xff04u,
	/*
	 * Timer counter (R/W)
	 *
	 * This timer is incremented by a clock frequency specified by the TAC
	 * register ($FF07).
	 * The timer generates an interrupt when it overflows.
	 */
	SPECIAL_REGISTER_TIMA = 0xff05u,
	/*
	 * Timer Modulo (R/W)
	 *
	 * When the TIMA overflows, this data will be loaded.
	 */
	SPECIAL_REGISTER_TMA = 0xff06u,
	/*
	 * Timer Control (R/W)
	 *
	 * Bit 2 - Timer Stop
	 *     0: Stop Timer
	 *     1: Start Timer
	 * Bits 1+0 - Input Clock Select
	 *     00: 4.096 KHz   (~4.194 KHz SGB)
	 *     01: 262.144 KHz (~268.4 KHz SGB)
	 *     10: 65.536 KHz  (~67.11 KHz SGB)
	 *     11: 16.384 KHz  (~16.78 KHz SGB)
	 */
	SPECIAL_REGISTER_TAC = 0xff07u,
	/*
	 * Interrupt Flag (R/W)
	 *
	 * Bit 4: Transition from High to Low of Pin number P10-P13
	 * Bit 3: Serial I/O transfer complete
	 * Bit 2: Timer Overflow
	 * Bit 1: LCDC (see STAT)
	 * Bit 0: V-Blank
	 *
	 * The priority and jump address for the above 5
	 *
	 * Interrupt        | Priority | Start Address
	 * -----------------+----------+--------------------------------------
	 * V-Blank          |     1    | $0040
	 * LCDC Status      |     2    | $0048 - Modes 0, 1, 2 LYC=LY coincide
	 *                  |          |         (selectable)
	 * Timer Overflow   |     3    | $0050
	 * Serial Transfer  |     4    | $0058 - when transfer is complete
	 * Hi-Lo of P10-P13 |     5    |
	 *
	 * When more than 1 interrupts occur at the same time only the interrupt
	 * with the highest priority can be acknowledged.
	 * When an interrupt is used a '0' should be stored in the IF register
	 * before the IE register is set.
	 */
	SPECIAL_REGISTER_IF = 0xff0fu,
	/*
	 * Sound Mode 1 register, Sweep register (R/W)
	 *
	 * Bit 6-4 - Sweep Time
	 * Bit 3   - Sweep Increase/Decrease
	 *        0: Addition (frequency increases)
	 *        1: Subtraction (frequency decreases)
	 * Bit 2-0 - Number of sweep shift (n: 0-7)
	 *
	 * Sweep Time:
	 *      000: sweep off - no freq change
	 *      001: 7.8 ms (1/128Hz)
	 *      010: 15.6 ms (2/128Hz)
	 *      011: 23.4 ms (3/128Hz)
	 *      100: 31.3 ms (4/128Hz)
	 *      101: 39.1 ms (5/128Hz)
	 *      110: 46.9 ms (6/128Hz)
	 *      111: 54.7 ms (7/128Hz)
	 *
	 * The change of frequency (NR13,NR14) at each shift is calculated by
	 * the following formula where X(0) is initial freq & X(t-1) is last
	 * freq:
	 *
	 *   X(t) = X(t-1) +/- X(t-1)/2^n
	 */
	SPECIAL_REGISTER_NR10 = 0xff10u,
	/*
	 * Sound Mode 1 register, Sound length/Wave pattern duty (R/W)
	 *
	 * Only bits 7-6 can be read.
	 *
	 * Bit 7-6 - Wave pattern duty
	 * Bit 5-0 - Sound length data (t1: 0-63)
	 *
	 * Wave Duty: (default: 10)
	 *            00: 12.5% ( _--------_--------_-------- )
	 *            01: 25%   ( __-------__-------__------- )
	 *            10: 50%   ( ____-----____-----____----- )
	 *            11: 75%   ( ______---______---______--- )
	 *
	 * Sound Length = (64-t1)*(1/256) seconds
	 */
	SPECIAL_REGISTER_NR11 = 0xff11u,
	/*
	 * Sound Mode 1 register, Envelope (R/W)
	 *
	 * Bit 7-4 - Initial volume of envelope
	 * Bit 3 - Envelope UP/DOWN
	 *         0: Attenuate
	 *         1: Amplify
	 * Bit 2-0 - Number of envelope sweep
	 *           (n: 0-7) (If zero, stop envelope operation.)
	 *
	 * Initial volume of envelope is from 0 to $F.
	 * Zero being no sound.
	 *
	 * Length of 1 step = n*(1/64) seconds
	 */
	SPECIAL_REGISTER_NR12 = 0xff12u,
	/*
	 * Sound Mode 1 register, Frequency lo (W)
	 *
	 * Lower 8 bits of 11 bit frequency (x).
	 * Next 3 bit are in NR 14 ($FF14)
	 */
	SPECIAL_REGISTER_NR13 = 0xff13u,
	/*
	 * Sound Mode 1 register, Frequency hi (R/W)
	 *
	 * Only bit 6 can be read
	 * Bit 7 - Initial (when set, sound restarts)
	 * Bit 6 - Counter/consecutive selection
	 * Bit 2-0 - Frequency's higher 3 bits (x)
	 *
	 * Frequency = 4194304 / (32 * (2048 - x)) Hz
	 *           = 131072 / (2048 - x) Hz
	 *
	 * Counter/consecutive Selection
	 * 0 = Regardless of the length data in NR21 sound can be produced
	 *     consecutively.
	 * 1 = Sound is generated during the time period set by the length data
	 *     in NR21.
	 *     After this period the sound 2 ON flag (bit 1 of NR52) is reset.
	 */
	SPECIAL_REGISTER_NR14 = 0xff14u,
	/*
	 * Sound Mode 2 register, Sound Length; Wave Pattern Duty (R/W)
	 *
	 * Only bits 7-6 can be read.
	 *
	 * Bit 7-6 - Wave pattern duty
	 * Bit 5-0 - Sound length data (t1: 0-63)
	 *
	 * Wave Duty: (default: 10)
	 *            00: 12.5% ( _--------_--------_-------- )
	 *            01: 25%   ( __-------__-------__------- )
	 *            10: 50%   ( ____-----____-----____----- )
	 *            11: 75%   ( ______---______---______--- )
	 *
	 * Sound Length = (64-t1)*(1/256) seconds
	 */
	SPECIAL_REGISTER_NR21 = 0xff16u,
	/*
	 * Sound Mode 2 register, envelope (R/W)
	 *
	 * Bit 7-4 - Initial volume of envelope
	 * Bit 3 - Envelope UP/DOWN
	 *         0: Attenuate
	 *         1: Amplify
	 * Bit 2-0 - Number of envelope sweep
	 *           (n: 0-7) (If zero, stop envelope operation.)
	 *
	 * Initial volume of envelope is from 0 to $F.
	 * Zero being no sound.
	 *
	 * Length of 1 step = n*(1/64) seconds
	 */
	SPECIAL_REGISTER_NR22 = 0xff17u,
	/*
	 * Sound Mode 2 register, frequency lo data (W)
	 *
	 * Frequency's lower 8 bits of 11 bit data (x).
	 * Next 3 bits are in NR 14 ($FF19).
	 */
	SPECIAL_REGISTER_NR23 = 0xff18u,
	/*
	 * Sound Mode 2 register, frequency hi data (R/W)
	 *
	 * Only bit 6 can be read
	 * Bit 7 - Initial (when set, sound restarts)
	 * Bit 6 - Counter/consecutive selection
	 * Bit 2-0 - Frequency's higher 3 bits (x)
	 *
	 * Frequency = 4194304 / (32 * (2048 - x)) Hz
	 *           = 131072 / (2048 - x) Hz
	 *
	 * Counter/consecutive Selection
	 * 0 = Regardless of the length data in NR21 sound can be produced
	 *     consecutively.
	 * 1 = Sound is generated during the time period set by the length data
	 *     in NR21.
	 *     After this period the sound 2 ON flag (bit 1 of NR52) is reset.
	 */
	SPECIAL_REGISTER_NR24 = 0xff19u,
	/*
	 * Sound Mode 3 register, Sound on/off (R/W)
	 *
	 * Only bit 7 can be read
	 *
	 * Bit 7 - Sound OFF
	 *         0: Sound 3 output stop
	 *         1: Sound 3 output OK
	 */
	SPECIAL_REGISTER_NR30 = 0xff1au,
	/*
	 * Sound Mode 3 register, sound length (R/W)
	 *
	 * Bit 7-0 - Sound length (t1: 0 - 255)
	 *
	 * Sound Length = (256-t1)*(1/2) seconds
	 */
	SPECIAL_REGISTER_NR31 = 0xff1bu,
	/*
	 * Sound Mode 3 register, Select output level (R/W)
	 *
	 * Only bits 6-5 can be read
	 *
	 * Bit 6-5 - Select output level
	 *           00: Mute
	 *           01: Produce Wave Pattern RAM Data as it is(4 bit length)
	 *           10: Produce Wave Pattern RAM data shifted once to the
	 *               RIGHT (1/2) (4 bit length)
	 *           11: Produce Wave Pattern RAM data shifted twice to the
	 *               RIGHT (1/4) (4 bit length)
	 *
	 * Wave Pattern RAM is located from $FF30-$FF3f.
	 */
	SPECIAL_REGISTER_NR32 = 0xff1cu,
	/*
	 * Sound Mode 3 register, frequency's lower data (W)
	 *
	 * Lower 8 bits of an 11 bit frequency (x).
	 */
	SPECIAL_REGISTER_NR33 = 0xff1du,
	/*
	 * Sound Mode 3 register, frequency's higher data (R/W)
	 *
	 * Only bit 6 can be read.
	 *
	 * Bit 7 - Initial (when set,sound restarts)
	 * Bit 6 - Counter/consecutive flag
	 * Bit 2-0 - Frequency's higher 3 bits (x).
	 *
	 * Frequency = 4194304/(64*(2048-x)) Hz
	 *           = 65536/(2048-x) Hz
	 *
	 * Counter/consecutive Selection
	 *  0 = Regardless of the length data in NR31 sound can be produced
	 *      consecutively.
	 *  1 = Sound is generated during the time period set by the length data
	 *      in NR31.
	 *      After this period the sound 3 ON flag (bit 2 of NR52) is reset.
	 */
	SPECIAL_REGISTER_NR34 = 0xff1eu,
	/*
	 * Sound Mode 4 register, sound length (R/W)
	 *
	 * Bit 5-0 - Sound length data (t1: 0-63)
	 *
	 * Sound Length = (64-t1)*(1/256) seconds
	 */
	SPECIAL_REGISTER_NR41 = 0xff20u,
	/*
	 * Sound Mode 4 register, envelope (R/W)
	 *
	 * Bit 7-4 - Initial volume of envelope
	 * Bit 3 - Envelope UP/DOWN
	 *         0: Attenuate
	 *         1: Amplify
	 * Bit 2-0 - Number of envelope sweep (n: 0-7) (If zero, stop
	 *           envelope operation.)
	 *
	 * Initial volume of envelope is from 0 to $F.
	 * Zero being no sound.
	 *
	 * Length of 1 step = n*(1/64) seconds
	 */
	SPECIAL_REGISTER_NR42 = 0xff21u,
	/*
	 * Sound Mode 4 register, polynomial counter (R/W)
	 *
	 * Bit 7-4 - Selection of the shift clock frequency of the polynomial
	 *           counter
	 * Bit 3 - Selection of the polynomial counter's step
	 * Bit 2-0 - Selection of the dividing ratio of frequencies:
	 *           000: f * 1/2^3 * 2
	 *           001: f * 1/2^3 * 1
	 *           010: f * 1/2^3 * 1/2
	 *           011: f * 1/2^3 * 1/3
	 *           100: f * 1/2^3 * 1/4
	 *           101: f * 1/2^3 * 1/5
	 *           110: f * 1/2^3 * 1/6
	 *           111: f * 1/2^3 * 1/7
	 *           f = 4.194304 Mhz
	 *
	 * Selection of the polynomial counter step:
	 * 0: 15 steps
	 * 1: 7 steps
	 *
	 * Selection of the shift clock frequency of the polynomial counter:
	 * 0000: dividing ratio of frequencies * 1/2
	 * 0001: dividing ratio of frequencies * 1/2^2
	 * 0010: dividing ratio of frequencies * 1/2^3
	 * 0011: dividing ratio of frequencies * 1/2^4
	 *          :                          :
	 *          :                          :
	 *          :                          :
	 * 0101: dividing ratio of frequencies * 1/2^14
	 * 1110: prohibited code
	 * 1111: prohibited code
	 */
	SPECIAL_REGISTER_NR43 = 0xff22u,
	/*
	 * Sound Mode 4 register, counter/consecutive; inital (R/W)
	 *
	 * Only bit 6 can be read.
	 *
	 * Bit 7 - Initial (when set, sound restarts)
	 * Bit 6 - Counter/consecutive selection
	 *
	 * Counter/consecutive Selection
	 *  0 = Regardless of the length data in NR41 sound can be produced
	 *      consecutively.
	 *  1 = Sound is generated during the time period set by the length data
	 *      in NR41.
	 *      After this period the sound 4 ON flag (bit 3 of NR52) is reset.
	 */
	SPECIAL_REGISTER_NR44 = 0xff23u,
	/*
	 * Channel control / ON-OFF / Volume (R/W)
	 *
	 * Bit 7 - Vin->SO2 ON/OFF
	 * Bit 6-4 - SO2 output level (volume) (# 0-7)
	 * Bit 3 - Vin->SO1 ON/OFF
	 * Bit 2-0 - SO1 output level (volume) (# 0-7)
	 *
	 * Vin->SO1 (Vin->SO2)
	 *
	 * By synthesizing the sound from sound 1 through 4, the voice input
	 * from Vin terminal is put out.
	 *  0: no output
	 *  1: output OK
	 */
	SPECIAL_REGISTER_NR50 = 0xff24u,
	/*
	 * Selection of Sound output terminal (R/W)
	 *
	 * Bit 7 - Output sound 4 to S02 terminal
	 * Bit 6 - Output sound 3 to S02 terminal
	 * Bit 5 - Output sound 2 to S02 terminal
	 * Bit 4 - Output sound 1 to S02 terminal
	 * Bit 3 - Output sound 4 to S01 terminal
	 * Bit 2 - Output sound 3 to S01 terminal
	 * Bit 1 - Output sound 2 to S01 terminal
	 * Bit 0 - Output sound 1 to S01 terminal
	 */
	SPECIAL_REGISTER_NR51 = 0xff25u,
	/*
	 * Sound on/off (R/W)
	 *
	 * Bit 7 - All sound on/off
	 *         0: stop all sound circuits
	 *         1: operate all sound circuits
	 * Bit 3 - Sound 4 ON flag
	 * Bit 2 - Sound 3 ON flag
	 * Bit 1 - Sound 2 ON flag
	 * Bit 0 - Sound 1 ON flag
	 * Bits 0 - 3 of this register are meant to be status bits to be read.
	 * Writing to these bits does NOT enable/disable sound.
	 *
	 * If your GB programs don't use sound then write $00 to this register
	 * to save 16% or more on GB power consumption.
	 */
	SPECIAL_REGISTER_NR52 = 0xff26u,
	/* FF30 - FF3F (Wave Pattern RAM) */
	/*
	 * Waveform storage for arbitrary sound data
	 *
	 * This storage area holds 32 4-bit samples that are played back upper
	 * 4 bits first.
	 */
	SPECIAL_REGISTER_WPRAM0,
	SPECIAL_REGISTER_WPRAM1,
	SPECIAL_REGISTER_WPRAM2,
	SPECIAL_REGISTER_WPRAM3,
	SPECIAL_REGISTER_WPRAM4,
	SPECIAL_REGISTER_WPRAM5,
	SPECIAL_REGISTER_WPRAM6,
	SPECIAL_REGISTER_WPRAM7,
	SPECIAL_REGISTER_WPRAM8,
	SPECIAL_REGISTER_WPRAM9,
	SPECIAL_REGISTER_WPRAMA,
	SPECIAL_REGISTER_WPRAMB,
	SPECIAL_REGISTER_WPRAMC,
	SPECIAL_REGISTER_WPRAMD,
	SPECIAL_REGISTER_WPRAME,
	SPECIAL_REGISTER_WPRAMF,
	/* LCD Control (R/W) */
	SPECIAL_REGISTER_LCDC = 0xff40u,
	/*
	 * LCDC Status (R/W)
	 * Bits 6-3 - Interrupt Selection By LCDC Status
	 * Bit 6 - LYC=LY Coincidence (Selectable)
	 * Bit 5 - Mode 10
	 * Bit 4 - Mode 01
	 * Bit 3 - Mode 00
	 *      0: Non Selection
	 *      1: Selection
	 * Bit 2 - Coincidence Flag
	 *       0: LYC not equal to LCDC LY
	 *       1: LYC = LCDC LY
	 * Bit 1-0 - Mode Flag
	 *       00: During H-Blank
	 *       01: During V-Blank
	 *       10: During Searching OAM-RAM
	 *       11: During Transfering Data to LCD Drive
	 */
	SPECIAL_REGISTER_STAT = 0xff41u,
	/*
	 * Scroll Y (R/W)
	 * 8 Bit value $00-$FF to scroll BG Y screen position.
	 */
	SPECIAL_REGISTER_SCY = 0xff42u,
	/*
	 * Scroll X (R/W)
	 * 8 Bit value $00-$FF to scroll BG X screen position.
	 */
	SPECIAL_REGISTER_SCX = 0xff43u,
	/*
	 * LCDC Y-Coordinate (R)
	 * The LY indicates the vertical line to which the present data is
	 * transferred to the LCD Driver.
	 * The LY can take on any value between 0 through 153.
	 * The values between 144 and 153 indicate the V-Blank period.
	 * Writing will reset the counter.
	 */
	SPECIAL_REGISTER_LY = 0xff44u,
	/*
	 * LY Compare (R/W)
	 * The LYC compares itself with the LY.
	 * If the values are the same it causes the STAT to set the coincident
	 * flag.
	 */
	SPECIAL_REGISTER_LYC = 0xff45u,
	/*
	 * DMA Transfer and Start Address (W)
	 *
	 * The DMA Transfer (40*28 bit) from internal ROM or RAM ($0000-$F19F)
	 * to the OAM (address $FE00-$FE9F) can be performed.
	 * It takes 160 microseconds for the transfer.
	 *
	 * 40*28 bit = #140 or #$8C. As you can see, it only transfers $8C bytes
	 * of data. OAM data is $A0 bytes long, from $0-$9F.
	 *
	 * But if you examine the OAM data you see that 4 bits are not in use.
	 *
	 * 40*32 bit = #$A0, but since 4 bits for each OAM is not used it's
	 * 40*28 bit.
	 *
	 * It transfers all the OAM data to OAM RAM.
	 *
	 * The DMA transfer start address can be designated every $100 from
	 * address $0000-$F100. That means $0000, $0100, $0200, $0300....
	 *
	 * As can be seen by looking at register $FF41 Sprite RAM
	 * ($FE00 - $FE9F) is not always available.
	 * A simple routine that many games use to write data to Sprite memory
	 * is shown below. Since it copies data to the sprite RAM at the
	 * appropriate times it removes that responsibility from the main
	 * program.
	 * All of the memory space, except high RAM ($FF80-$FFFE), is not
	 * accessible during DMA.
	 * Because of this, the routine below must be copied & executed in high
	 * ram.
	 * It is usually called from a V-blank Interrupt.
	 */
	SPECIAL_REGISTER_DMA = 0xff46u,
	/*
	 * BG & Window Palette Data (R/W)
	 *
	 * Bit 7-6 - Data for Dot Data 11
	 *           (Normally darkest color)
	 * Bit 5-4 - Data for Dot Data 10
	 * Bit 3-2 - Data for Dot Data 01
	 * Bit 1-0 - Data for Dot Data 00
	 *           (Normally lightest color)
	 * This selects the shade of grays to use for the background (BG) &
	 * window pixels.
	 * Since each pixel uses 2 bits, the corresponding shade will be
	 * selected from here.
	 */
	SPECIAL_REGISTER_BGP = 0xff47u,
	/*
	 * Object Palette 0 Data (R/W)
	 *
	 * This selects the colors for sprite palette 0.
	 * It works exactly as BGP ($FF47) except each each value of 0 is
	 * transparent.
	 */
	SPECIAL_REGISTER_OBP0 = 0xff48u,
	/*
	 * Object Palette 1 Data (R/W)
	 *
	 * This Selects the colors for sprite palette 1.
	 * It works exactly as OBP0 ($FF48).
	 * See BGP for details.
	 */
	SPECIAL_REGISTER_OBP1 = 0xff49u,
	/*
	 * Window Y Position (R/W)
	 *
	 * 0 <= WY <= 1430
	 * WY must be greater than or equal to 0 and must be less than or equal
	 * to 143 for window to be visible.
	 */
	SPECIAL_REGISTER_WY = 0xff4au,
	/*
	 * Window X Position (R/W)
	 *
	 * 0 <= WX <= 166
	 * WX must be greater than or equal to 0 and must be less than or equal
	 * to 166 for window to be visible.
	 *
	 * WX is offset from absolute screen coordinates by 7.
	 * Setting the window to WX=7, WY=0 will put the upper left corner of
	 * the window at absolute screen coordinates 0,0.
	 *
	 * Lets say WY = 70 and WX = 87.
	 * The window would be positioned as so:
	 *
	 *      0                  80               159
	 *      ______________________________________
	 *   0 |                                      |
	 *     |                   |                  |
	 *     |                                      |
	 *     |         Background Display           |
	 *     |               Here                   |
	 *     |                                      |
	 *     |                                      |
	 *  70 |         -         +------------------|
	 *     |                   | 80,70            |
	 *     |                   |                  |
	 *     |                   |  Window Display  |
	 *     |                   |       Here       |
	 *     |                   |                  |
	 *     |                   |                  |
	 * 143 |___________________|__________________|
	 *
	 * OBJ Characters (Sprites) can still enter the window.
	 * None of the window colors are transparent so any background tiles
	 * under the window are hidden.
	 */
	SPECIAL_REGISTER_WX = 0xff4bu,

	/*
	 * Zone of memory which must be used during DMA transfers, see
	 * SPECIAL_REGISTER_DMA
	 */
	SPECIAL_REGISTER_HIGH_RAM_START = 0xff80u,
	SPECIAL_REGISTER_HIGH_RAM_END = 0xfffeu,
	/*
	 * Interrupt Enable (R/W)
	 * Bit 4: Transition from High to Low of Pin number P10-P13.
	 * Bit 3: Serial I/O transfer complete
	 * Bit 2: Timer Overflow
	 * Bit 1: LCDC (see STAT)
	 * Bit 0: V-Blank
	 *
	 * 0: disable
	 * 1: enable
	 */
	SPECIAL_REGISTER_IE = 0xffffu,

	SPECIAL_REGISTER_LAST = SPECIAL_REGISTER_IE,
};

const char *special_register_to_str(enum special_register reg);
enum special_register special_register_from_string(const char *str);

#endif /* INCLUDE_SPECIAL_REGISTERS_H_ */
