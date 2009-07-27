set $initialized = 0

echo + symbol-file obj/kernel\n
symbol-file obj/kernel

define 16bitmode
  echo + set architecture i8086\n
  set architecture i8086
  undisplay
  echo + display/i $cs*16+$eip\n
  display/i $cs*16+$eip
  if $initialized
    echo + x/i $cs*16+$eip\n
    x/i $cs*16+$eip
  end
end

define 32bitmode
  echo + set architecture i386\n
  set architecture i386
  undisplay
  echo + display/i $pc\n
  display/i $pc
  if $initialized
    echo + x/i $pc\n
    x/i $pc
  end
end

# Start in 16-bit mode by default.
16bitmode

echo + target remote localhost:1234\n
target remote localhost:1234

set $initialized = 1
