
int main() {
	object pdf=PDF.pdf("/tmp/something.pdf");
	pdf->begin_page(300.00,300.00);        
	int font=pdf->findfont("Helvetica-Bold","host",0);
	pdf->setfont(font,24);
	pdf->show("asdasjdkalsjdkasjdasdgashda jhdgahsdg as");
	pdf->end_page();
	pdf->close();
}
