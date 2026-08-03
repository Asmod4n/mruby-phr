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
  FileUtils.mkdir_p "#{spec.dir}/include"
  FileUtils.cp "#{picohttpparser_src}/picohttpparser.h", "#{spec.dir}/include/"

  spec.objs += %W(
    #{picohttpparser_src}/picohttpparser.c
  ).map { |f| f.relative_path_from(dir).pathmap("#{build_dir}/%X#{spec.exts.object}" ) }
end
