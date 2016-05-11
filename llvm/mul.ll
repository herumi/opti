define i32 @extract64(i64 %x, i64 %shift) {
	%t0 = lshr i64 %x, %shift
	%t1 = trunc i64 %t0 to i32
	ret i32 %t1
}
define i64 @mul32x32(i32 %x, i32 %y) {
	%x0 = zext i32 %x to i64
	%y0 = zext i32 %y to i64
	%z = mul i64 %x0, %y0
	ret i64 %z
}
define i64 @mulp32x32(i32 *%px, i32 %y, i32 %i)
{
	%p = getelementptr i32* %px, i32 %i
	%x = load i32* %p
	%xy = call i64 @mul32x32(i32 %x, i32 %y)
	ret i64 %xy
}

define void @storePos(i32 *%p, i32 %v, i32 %i)
{
	%pp = getelementptr i32 *%p, i32 %i
	store i32 %v, i32* %pp
	ret void
}

define void @Xmul128x32(i32 *%pz, i32 *%px, i32 %y)
{
	%x0y = call i64 @mulp32x32(i32 *%px, i32 %y, i32 0)
	%L0 = call i32 @extract64(i64 %x0y, i64 0)
	%H0 = call i32 @extract64(i64 %x0y, i64 32)
	call void @storePos(i32 *%pz, i32 %L0, i32 0)

	%x1y = call i64 @mulp32x32(i32 *%px, i32 %y, i32 1)
	%L1 = call i32 @extract64(i64 %x1y, i64 0)
	%H1 = call i32 @extract64(i64 %x1y, i64 32)
	call void @storePos(i32 *%pz, i32 %L1, i32 1)

	%x2y = call i64 @mulp32x32(i32 *%px, i32 %y, i32 2)
	%L2 = call i32 @extract64(i64 %x2y, i64 0)
	%H2 = call i32 @extract64(i64 %x2y, i64 32)
	call void @storePos(i32 *%pz, i32 %L2, i32 2)

	%x3y = call i64 @mulp32x32(i32 *%px, i32 %y, i32 3)
	%L3 = call i32 @extract64(i64 %x3y, i64 0)
	%H3 = call i32 @extract64(i64 %x3y, i64 32)
	call void @storePos(i32 *%pz, i32 %L3, i32 3)

	%L1e = zext i32 %L1 to i128
	%L2e = zext i32 %L2 to i128
	%L3e = zext i32 %L3 to i128
	%L2e1 = shl i128 %L2e, 32
	%L3e1 = shl i128 %L3e, 64
	%Lt0 = or i128 %L1e, %L2e1
	%Lt1 = or i128 %Lt0, %L3e1

	%H0e = zext i32 %H0 to i128
	%H1e = zext i32 %H1 to i128
	%H2e = zext i32 %H2 to i128
	%H3e = zext i32 %H3 to i128
	%H1e1 = shl i128 %H1e, 32
	%H2e1 = shl i128 %H2e, 64
	%H3e1 = shl i128 %H3e, 96
	%Ht0 = or i128 %H0e, %H1e1
	%Ht1 = or i128 %Ht0, %H2e1
	%Ht2 = or i128 %Ht1, %H3e1
	%t = add i128 %Ht2, %Lt1

	%pz1 = getelementptr i32* %pz, i32 1
	%pz2 = bitcast i32* %pz1 to i128*

	store i128 %t, i128* %pz2
	
	ret void
}
