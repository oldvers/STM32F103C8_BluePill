#ifndef __CDC_DEFS_H__
#define __CDC_DEFS_H__

///* CDC Interface Details Definitions */
//#define CDC_IF_CLASS_CDC                   (0x0A) /* CDC */
//#define CDC_IF_SUBCLASS_NONE               (0x00) /* None */
//#define CDC_IF_SUBCLASS_ACM                (0x02) /* Abstract Control Model */
//#define CDC_IF_PROTOCOL_NONE               (0x00) /* None */
//#define CDC_IF_PROTOCOL_AT_CMD             (0x01) /* Common AT commands */

///* CDC Class Specific Interface Descriptor Type */
//#define CDC_CS_INTERFACE_DESC_TYPE         (0x24)

///* CDC Functional Descriptor Subtype */
//#define CDC_HEADER_FNC_DESC_SUBTYPE        (0x00) /* Header */
//#define CDC_CALL_MGMT_FNC_DESC_SUBTYPE     (0x01) /* Call Management */
//#define CDC_ACM_FNC_DESC_SUBTYPE           (0x02) /* Abstract Control Model */
//#define CDC_UNION_FNC_DESC_SUBTYPE         (0x06) /* Union */

///* CDC Functional Descriptor: Sum of Sizes */
//#define CDC_FNC_DESC_SUM_SIZE              (19)

///* CDC Class Requests */
//#define CDC_REQ_SEND_ENCAPSULATED_COMMAND  (0x00)
//#define CDC_REQ_GET_ENCAPSULATED_RESPONSE  (0x01)
//#define CDC_REQ_SET_COMM_FEATURE           (0x02)
//#define CDC_REQ_GET_COMM_FEATURE           (0x03)
//#define CDC_REQ_CLEAR_COMM_FEATURE         (0x04)
//#define CDC_REQ_SET_LINE_CODING            (0x20)
//#define CDC_REQ_GET_LINE_CODING            (0x21)
//#define CDC_REQ_SET_CONTROL_LINE_STATE     (0x22)
//#define CDC_REQ_SEND_BREAK                 (0x23)
//#define CDC_REQ_NO_CMD                     (0xFF)

///* CDC Class Notifications */
//#define CDC_NTF_SERIAL_STATE               (0x20)

#endif /* __CDC_DEFS_H__ */
