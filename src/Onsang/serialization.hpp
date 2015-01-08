/**
@file
@brief Serialization definitions.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

#include <Onsang/config.hpp>

#include <Hord/serialization.hpp>

namespace Onsang {

// Forward declarations

using Hord::const_safe;

using Hord::SerializerErrorCode;
using Hord::SerializerError;

using Hord::InputSerializer;
using Hord::OutputSerializer;

using Hord::ser_result_type;

using Hord::ser_tag_serialize;
using Hord::ser_tag_read;
using Hord::ser_tag_write;

using Hord::get_ser_error_name;

using Hord::make_input_serializer;
using Hord::make_output_serializer;

} // namespace Onsang
