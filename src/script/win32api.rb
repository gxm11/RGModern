# -*- ruby -*-
# frozen_string_literal: true

# for backward compatibility
# warn 'Win32API is deprecated after Ruby 1.9.1; use fiddle directly instead', uplevel: 2

# require 'fiddle/import'
load_script 'fiddle/closure.rb'
load_script 'fiddle/function.rb'
load_script 'fiddle/version.rb'
load_script 'fiddle.rb'
load_script 'fiddle/value.rb'
load_script 'fiddle/pack.rb'
load_script 'fiddle/struct.rb'
load_script 'fiddle/cparser.rb'
load_script 'fiddle/import.rb'

class Win32API
  DLL = {}
  TYPEMAP = { '0' => Fiddle::TYPE_VOID, 'S' => Fiddle::TYPE_VOIDP, 'I' => Fiddle::TYPE_LONG }
  POINTER_TYPE = Fiddle::SIZEOF_VOIDP == Fiddle::SIZEOF_LONG_LONG ? 'q*' : 'l!*'

  WIN32_TYPES = 'VPpNnLlIi'
  DL_TYPES = '0SSI'

  def initialize(dllname, func, import, export = '0', calltype = :stdcall)
    @proto = [import].join.tr(WIN32_TYPES, DL_TYPES).sub(/^(.)0*$/, '\1')
    import = @proto.chars.map { |win_type| TYPEMAP[win_type.tr(WIN32_TYPES, DL_TYPES)] }
    export = TYPEMAP[export.tr(WIN32_TYPES, DL_TYPES)]
    calltype = Fiddle::Importer.const_get(:CALL_TYPE_TO_ABI)[calltype]

    handle = DLL[dllname] ||=
      begin
        Fiddle.dlopen(dllname)
      rescue Fiddle::DLError
        raise unless File.extname(dllname).empty?

        Fiddle.dlopen(dllname + '.dll')
      end

    @func = Fiddle::Function.new(handle[func], import, export, calltype)
  rescue Fiddle::DLError => e
    raise LoadError, e.message, e.backtrace
  end

  def call(*args)
    import = @proto.split('')
    args.each_with_index do |x, i|
      args[i], = [x == 0 ? nil : x].pack('p').unpack(POINTER_TYPE) if import[i] == 'S'
      args[i], = [x].pack('I').unpack('i') if import[i] == 'I'
    end
    ret, = @func.call(*args)
    ret || 0
  end

  alias Call call
end
