MRuby::Gem::Specification.new('mruby-phr') do |spec|
  spec.license = 'Apache-2'
  spec.author  = 'Hendrik Beskow'
  spec.summary = 'mruby gem for picohttpparser'

  picohttpparser_src = "#{spec.dir}/deps/picohttpparser"
  spec.cc.include_paths << "#{picohttpparser_src}"

  # Expose picohttpparser's public header to dependent gems, the same
  # way mruby-wslay exposes wslay's and mruby-io-uring exposes
  # liburing's: the include_paths above only reach this gem's own
  # compilation, but every mrbgem's include/ is on every other gem's
  # search path, so this is what lets a dependent gem parse requests
  # with phr_parse_request directly in its own C++ instead of going
  # through the Ruby Phr object. Build output, not tracked source -
  # see .gitignore.
  #
  # Copied only when the bytes differ. FileUtils.cp writes the file on
  # every `rake compile` and gives it a new mtime, so every dependent
  # gem that includes it recompiled on every single run, on a tree
  # where nothing had changed. Measured in webmachine-mruby: three
  # objects per idle build.
  #
  # The comparison is on CONTENT, not on mtime: a picohttpparser
  # update writes the new bytes and the rebuild follows, which is what
  # has to keep working. FileUtils.identical? reads both files, so a
  # changed header is never mistaken for an unchanged one.
  FileUtils.mkdir_p "#{spec.dir}/include"
  phr_header = "#{spec.dir}/include/picohttpparser.h"
  phr_header_src = "#{picohttpparser_src}/picohttpparser.h"
  unless File.exist?(phr_header) && FileUtils.identical?(phr_header_src, phr_header)
    FileUtils.cp phr_header_src, phr_header
  end

  spec.objs += %W(
    #{picohttpparser_src}/picohttpparser.c
  ).map { |f| f.relative_path_from(dir).pathmap("#{build_dir}/%X#{spec.exts.object}" ) }
end
