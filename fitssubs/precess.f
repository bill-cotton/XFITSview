C Program to convert from B1950 to J2000 coordinates
C 94/02/22  J. J. Condon
C
      character*1 isgn
      write (*, *) 'btoj: program to precess from B1950 to J2000'
 10   continue
      write (*, *) 'enter B1950 coordinates as HH MM SS.SS +DD MM SS.S'
      write (*, *) 'or just hit "return" to quit.'
      read (*, 1010) ihra, imra, sra, isgn, idd, imd, sd
      if (isgn .eq. ' ') go to 100
      call btoj (ihra, imra, sra, isgn, idd, imd, sd)
      write (*, 1010) ihra, imra, sra, isgn, idd, imd, sd
      go to 10
 100  continue  
      stop
 1010 format (i2, 1x, i2, 1x, f5.2, 1x, a1, i2, 1x, i2, 1x, f4.1)
      end

      SUBROUTINE BTOJ (IHRA, IMRA, SRA, ISGN, IDD, IMD, SD)
C **********************************************************************
C                   CONVERTS B1950 RA, DEC TO J2000
C                   USING METHOD ON PAGE B42 OF 
C                   THE ASTRONOMICAL ALMANAC (1990 ED.).
C                   CALLING AND RETURNED ARGUMENTS:
C                   IHRA = HOURS OF RIGHT ASCENSION (INTEGER*4)
C                   IMRA = MINUTES OF RIGHT ASCENSION (INTEGER*4)
C                   SRA = SECONDS OF RIGHT ASCENSION (REAL*4)
C                   ISGN = SIGN OF DECLINATION (CHARACTER*1)
C                          '-' FOR NEGATIVE,
C                          ANYTHING ELSE FOR POSITIVE IN INPUT,
C                           '+' RETURNED FOR POSITIVE ON OUTPUT
C                   IDD = DEGREES OF DECLINATION (INTEGER*4)
C                   IMD = MINUTES OF DECLINATION (INTEGER*4)
C                   SD = SECONDS OF DECLINATION (REAL*4)
C                   REVISED 90/05/07 J. J. CONDON
C **********************************************************************
      CHARACTER*1 ISGN, MINUS, PLUS
      REAL*8 RA, DEC, RA0, DEC0, TEMP, R0TA
      REAL*8 SINA, COSA, SIND, COSD, RNORM, PI
      REAL*8 A(3), M(3,3), R0(3), R1(3), R(3)
      DATA MINUS /'-'/, PLUS /'+'/
      DATA A(1) /-1.62557D-6/, A(2) /-0.31919D-6/, A(3) /-0.13843D-6/
      DATA M(1,1) /+0.9999256782D0/, M(1,2) /-0.0111820611D0/
      DATA M(1,3) /-0.0048579477D0/, M(2,1) /+0.0111820610D0/
      DATA M(2,2) /+0.9999374784D0/, M(2,3) /-0.0000271765D0/
      DATA M(3,1) /+0.0048579479D0/, M(3,2) /-0.0000271474D0/
      DATA M(3,3) /+0.9999881997D0/
      DATA PI /3.1415926536D0/
C                   NOTE: ALL CALCULATIONS DOUBLE PRECISION
C                   FIRST CONVERT INPUT B1950 RA, DEC TO RADIANS
      TEMP = SRA
      RA0 = 3600 * IHRA + 60 * IMRA
      RA0 = (RA0 + TEMP) * PI / 43200.D0
      TEMP = SD
      DEC0 = 3600 * IDD + 60 * IMD
      DEC0 = (DEC0 + TEMP) * PI / 648000.D0
      IF (ISGN .EQ. MINUS) DEC0 = -DEC0
C                   THEN CONVERT B1950 RA, DEC TO CARTESIAN COORDINATES
      R0(1) = DCOS (RA0) * DCOS (DEC0)
      R0(2) = DSIN (RA0) * DCOS (DEC0)
      R0(3) = DSIN (DEC0)
C                   REMOVE ABERRATION E-TERMS      
C                   (R0TA = SCALAR PRODUCT OF R0 AND A)
      R0TA = R0(1) * A(1) + R0(2) * A(2) + R0(3) * A(3)
      DO 10 I = 1, 3
         R1(I) = R0(I) - A(I) + R0TA * R0(I)
   10    CONTINUE
C                   PRECESS FROM B1950 TO J2000
      DO 20 I = 1, 3
         R(I) = M(I,1) * R1(1) + M(I,2) * R1(2) + M(I,3) * R1(3)
   20    CONTINUE
C                   CONVERT J2000 CARTESIAN COORDINATES
C                   TO J2000 RA, DEC (RADIANS)
      RNORM = DSQRT (R(1) * R(1) + R(2) * R(2) + R(3) * R(3))
      SIND = R(3) / RNORM
      DEC = DASIN (SIND)
      COSD = DSQRT (1.D0 - SIND * SIND)
      COSA = R(1) / (RNORM * COSD)
      SINA = R(2) / (RNORM * COSD)
      IF (COSA .NE. 0.D0) RA = DATAN (SINA / COSA)
      IF (COSA .EQ. 0.D0) THEN
         IF (SINA .GT. 0.D0) RA = PI / 2.D0
         IF (SINA .LT. 0.D0) RA = 1.5D0 * PI
         END IF
C                   THEN CONVERT TO DECIMAL DEG OF DEC, HR OF RA,
C                   DETERMINE SIGN OF DECLINATION, AND
C                   RESOLVE 12H AMBIGUITY OF RA
      DEC = DEC * 180.D0 / PI
      IF (DEC .GE. 0.D0) ISGN = PLUS
      IF (DEC .LT. 0.D0) THEN
         ISGN = MINUS
         DEC = -DEC
         END IF
      RA = RA * 12.D0 / PI
      IF (COSA .LT. 0.D0) RA = RA + 12.D0
      IF (RA .LT. 0.D0) RA = RA + 24.D0
C                   FINALLY CONVERT J2000 RA, DEC TO UNITS OF ARGUMENT
      IDD = DEC
      TEMP = IDD
      DEC = 60.D0 * (DEC - TEMP)
      IMD = DEC
      TEMP = IMD
      DEC = 60.D0 * (DEC - TEMP)
      SD = DEC
      IHRA = RA
      TEMP = IHRA
      RA = 60.D0 * (RA - TEMP)
      IMRA = RA
      TEMP = IMRA
      RA = 60.D0 * (RA - TEMP)
      SRA = RA
      RETURN
      END
      SUBROUTINE JTOB (IHRA, IMRA, SRA, ISGN, IDD, IMD, SD)
C **********************************************************************
C                   CONVERTS J2000 RA, DEC TO B1950
C                   USING METHOD ON PAGE B43 OF 
C                   THE ASTRONOMICAL ALMANAC (1990 ED.).
C                   CALLING AND RETURNED ARGUMENTS:
C                   IHRA = HOURS OF RIGHT ASCENSION (INTEGER*4)
C                   IMRA = MINUTES OF RIGHT ASCENSION (INTEGER*4)
C                   SRA = SECONDS OF RIGHT ASCENSION (REAL*4)
C                   ISGN = SIGN OF DECLINATION (CHARACTER*1)
C                          '-' FOR NEGATIVE,
C                          ANYTHING ELSE FOR POSITIVE IN INPUT,
C                           '+' RETURNED FOR POSITIVE ON OUTPUT
C                   IDD = DEGREES OF DECLINATION (INTEGER*4)
C                   IMD = MINUTES OF DECLINATION (INTEGER*4)
C                   SD = SECONDS OF DECLINATION (REAL*4)
C                   REVISED 90/10/15 J. J. CONDON
C **********************************************************************
      CHARACTER*1 ISGN, MINUS, PLUS
      REAL*8 RA, DEC, RA0, DEC0, TEMP, STA
      REAL*8 SINA, COSA, SIND, COSD, RNORM, R1NORM, PI
      REAL*8 A(3), MINV(3,3), R0(3), R1(3), R(3), S(3), S1(3)
      DATA MINUS /'-'/, PLUS /'+'/
      DATA A(1) /-1.62557D-6/, A(2) /-0.31919D-6/, A(3) /-0.13843D-6/
      DATA MINV(1,1) /+0.9999256795D0/, MINV(1,2) /+0.0111814828D0/
      DATA MINV(1,3) /+0.0048590039D0/, MINV(2,1) /-0.0111814828D0/
      DATA MINV(2,2) /+0.9999374849D0/, MINV(2,3) /-0.0000271771D0/
      DATA MINV(3,1) /-0.0048590040D0/, MINV(3,2) /-0.0000271557D0/
      DATA MINV(3,3) /+0.9999881946D0/
      DATA PI /3.1415926536D0/
C                   NOTE: ALL CALCULATIONS DOUBLE PRECISION
C                   FIRST CONVERT INPUT J2000 RA, DEC TO RADIANS
      TEMP = SRA
      RA0 = 3600 * IHRA + 60 * IMRA
      RA0 = (RA0 + TEMP) * PI / 43200.D0
      TEMP = SD
      DEC0 = 3600 * IDD + 60 * IMD
      DEC0 = (DEC0 + TEMP) * PI / 648000.D0
      IF (ISGN .EQ. MINUS) DEC0 = -DEC0
C                   THEN CONVERT J2000 RA, DEC TO CARTESIAN COORDINATES
      R0(1) = DCOS (RA0) * DCOS (DEC0)
      R0(2) = DSIN (RA0) * DCOS (DEC0)
      R0(3) = DSIN (DEC0)
C                   PRECESS FROM J2000 TO B1950
      DO 10 I = 1, 3
         R1(I) = MINV(I,1) * R0(1) + MINV(I,2) * R0(2) + 
     *      MINV(I,3) * R0(3)
   10    CONTINUE
C                   INCLUDE ABERRATION E-TERMS      
      R1NORM = DSQRT (R1(1) * R1(1) + R1(2) * R1(2) + R1(3) * R1(3))     
      DO 20 I = 1, 3
         S1(I) = R1(I) / R1NORM
   20    CONTINUE
      DO 30 I = 1, 3
         S(I) = S1(I)
   30    CONTINUE
C                   THREE-STEP ITERATION FOR R
      DO 60 ITER = 1, 3
C                   (STA = SCALAR PRODUCT OF S AND A)
         STA = S(1) * A(1) + S(2) * A(2) + S(3) * A(3)
C                   CALCULATE OR RECALCULATE R
         DO 40 I = 1, 3
            R(I) = S1(I) + A(I)  - STA * S(I)
   40       CONTINUE
         RNORM = DSQRT (R(1) * R(1) + R(2) * R(2) + R(3) * R(3))
C                   CALCULATE OR RECALCULATE S
         DO 50 I = 1, 3
            S(I) = R(I) / RNORM
   50       CONTINUE
   60    CONTINUE
C                   CONVERT B1950 CARTESIAN COORDINATES (R-TRANSPOSE)
C                   TO B1950 RA, DEC (RADIANS)
      SIND = R(3) / RNORM
      DEC = DASIN (SIND)
      COSD = DSQRT (1.D0 - SIND * SIND)
      COSA = R(1) / (RNORM * COSD)
      SINA = R(2) / (RNORM * COSD)
      IF (COSA .NE. 0.D0) RA = DATAN (SINA / COSA)
      IF (COSA .EQ. 0.D0) THEN
         IF (SINA .GT. 0.D0) RA = PI / 2.D0
         IF (SINA .LT. 0.D0) RA = 1.5D0 * PI
         END IF
C                   THEN CONVERT TO DECIMAL DEG OF DEC, HR OF RA,
C                   DETERMINE SIGN OF DECLINATION, AND
C                   RESOLVE 12H AMBIGUITY OF RA
      DEC = DEC * 180.D0 / PI
      IF (DEC .GE. 0.D0) ISGN = PLUS
      IF (DEC .LT. 0.D0) THEN
         ISGN = MINUS
         DEC = -DEC
         END IF
      RA = RA * 12.D0 / PI
      IF (COSA .LT. 0.D0) RA = RA + 12.D0
      IF (RA .LT. 0.D0) RA = RA + 24.D0
C                   FINALLY CONVERT B1950 RA, DEC TO UNITS OF ARGUMENT
      IDD = DEC
      TEMP = IDD
      DEC = 60.D0 * (DEC - TEMP)
      IMD = DEC
      TEMP = IMD
      DEC = 60.D0 * (DEC - TEMP)
      SD = DEC
      IHRA = RA
      TEMP = IHRA
      RA = 60.D0 * (RA - TEMP)
      IMRA = RA
      TEMP = IMRA
      RA = 60.D0 * (RA - TEMP)
      SRA = RA
      RETURN
      END


      SUBROUTINE BTOJ (IHRA, IMRA, SRA, ISGN, IDD, IMD, SD)
C **********************************************************************
C                   CONVERTS B1950 RA, DEC TO J2000
C                   USING METHOD ON PAGE B42 OF 
C                   THE ASTRONOMICAL ALMANAC (1990 ED.).
C                   CALLING AND RETURNED ARGUMENTS:
C                   IHRA = HOURS OF RIGHT ASCENSION (INTEGER*4)
C                   IMRA = MINUTES OF RIGHT ASCENSION (INTEGER*4)
C                   SRA = SECONDS OF RIGHT ASCENSION (REAL*4)
C                   ISGN = SIGN OF DECLINATION (CHARACTER*1)
C                          '-' FOR NEGATIVE,
C                          ANYTHING ELSE FOR POSITIVE IN INPUT,
C                           '+' RETURNED FOR POSITIVE ON OUTPUT
C                   IDD = DEGREES OF DECLINATION (INTEGER*4)
C                   IMD = MINUTES OF DECLINATION (INTEGER*4)
C                   SD = SECONDS OF DECLINATION (REAL*4)
C                   REVISED 90/05/07 J. J. CONDON
C **********************************************************************
      CHARACTER*1 ISGN, MINUS, PLUS
      REAL*8 RA, DEC, RA0, DEC0, TEMP, R0TA
      REAL*8 SINA, COSA, SIND, COSD, RNORM, PI
      REAL*8 A(3), M(3,3), R0(3), R1(3), R(3)
      DATA MINUS /'-'/, PLUS /'+'/
      DATA A(1) /-1.62557D-6/, A(2) /-0.31919D-6/, A(3) /-0.13843D-6/
      DATA M(1,1) /+0.9999256782D0/, M(1,2) /-0.0111820611D0/
      DATA M(1,3) /-0.0048579477D0/, M(2,1) /+0.0111820610D0/
      DATA M(2,2) /+0.9999374784D0/, M(2,3) /-0.0000271765D0/
      DATA M(3,1) /+0.0048579479D0/, M(3,2) /-0.0000271474D0/
      DATA M(3,3) /+0.9999881997D0/
      DATA PI /3.1415926536D0/
C                   NOTE: ALL CALCULATIONS DOUBLE PRECISION
C                   FIRST CONVERT INPUT B1950 RA, DEC TO RADIANS
      TEMP = SRA
      RA0 = 3600 * IHRA + 60 * IMRA
      RA0 = (RA0 + TEMP) * PI / 43200.D0
      TEMP = SD
      DEC0 = 3600 * IDD + 60 * IMD
      DEC0 = (DEC0 + TEMP) * PI / 648000.D0
      IF (ISGN .EQ. MINUS) DEC0 = -DEC0
C                   THEN CONVERT B1950 RA, DEC TO CARTESIAN COORDINATES
      R0(1) = DCOS (RA0) * DCOS (DEC0)
      R0(2) = DSIN (RA0) * DCOS (DEC0)
      R0(3) = DSIN (DEC0)
C                   REMOVE ABERRATION E-TERMS      
C                   (R0TA = SCALAR PRODUCT OF R0 AND A)
      R0TA = R0(1) * A(1) + R0(2) * A(2) + R0(3) * A(3)
      DO 10 I = 1, 3
         R1(I) = R0(I) - A(I) + R0TA * R0(I)
   10    CONTINUE
C                   PRECESS FROM B1950 TO J2000
      DO 20 I = 1, 3
         R(I) = M(I,1) * R1(1) + M(I,2) * R1(2) + M(I,3) * R1(3)
   20    CONTINUE
C                   CONVERT J2000 CARTESIAN COORDINATES
C                   TO J2000 RA, DEC (RADIANS)
      RNORM = DSQRT (R(1) * R(1) + R(2) * R(2) + R(3) * R(3))
      SIND = R(3) / RNORM
      DEC = DASIN (SIND)
      COSD = DSQRT (1.D0 - SIND * SIND)
      COSA = R(1) / (RNORM * COSD)
      SINA = R(2) / (RNORM * COSD)
      IF (COSA .NE. 0.D0) RA = DATAN (SINA / COSA)
      IF (COSA .EQ. 0.D0) THEN
         IF (SINA .GT. 0.D0) RA = PI / 2.D0
         IF (SINA .LT. 0.D0) RA = 1.5D0 * PI
         END IF
C                   THEN CONVERT TO DECIMAL DEG OF DEC, HR OF RA,
C                   DETERMINE SIGN OF DECLINATION, AND
C                   RESOLVE 12H AMBIGUITY OF RA
      DEC = DEC * 180.D0 / PI
      IF (DEC .GE. 0.D0) ISGN = PLUS
      IF (DEC .LT. 0.D0) THEN
         ISGN = MINUS
         DEC = -DEC
         END IF
      RA = RA * 12.D0 / PI
      IF (COSA .LT. 0.D0) RA = RA + 12.D0
      IF (RA .LT. 0.D0) RA = RA + 24.D0
C                   FINALLY CONVERT J2000 RA, DEC TO UNITS OF ARGUMENT
      IDD = DEC
      TEMP = IDD
      DEC = 60.D0 * (DEC - TEMP)
      IMD = DEC
      TEMP = IMD
      DEC = 60.D0 * (DEC - TEMP)
      SD = DEC
      IHRA = RA
      TEMP = IHRA
      RA = 60.D0 * (RA - TEMP)
      IMRA = RA
      TEMP = IMRA
      RA = 60.D0 * (RA - TEMP)
      SRA = RA
      RETURN
      END
